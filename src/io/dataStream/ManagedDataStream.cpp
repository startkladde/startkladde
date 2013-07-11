#include "ManagedDataStream.h"

#include <QDebug>
#include <QTimer>

#include "src/io/dataStream/DataStream.h"

// FIXME seems like we'll need (at least) two independent state machines:
// What can happen is that the connection is terminated (state: error), and then
// a data timeout comes along (state: timeout, which is clearly wrong). Note
// that the open flag is basically a third state machine.

// Implementation notes:
//   * We want to be able to assign a new DataStream instance to this class.
//     Therefore, we have to store the open/closed state (as requested by the
//     user) here rather than relying on the state of DataStream.

// The meaning of the individual states:
//   * closed: the user has not requested the stream to be opened. All other
//     states imply that the user has requested the stream to be opened.
//   * opening: the connection is currently being established. This may be the
//     initial connect after opening the stream, or a reconnect after a
//     connection fails or is lost (see below).
//     In this state, errorType indicates the type of error (connection failed
//     or lost), if any, and errorMessage may contain an error message string.
//   * open: the connection has been established, but no data has been
//     received yet. This state does not indicate an error, it is always reached
//     after connecting successfully. If there is a data timeout after some data
//     has been received, streamDataTimeout is used (see below).
//   * ok: the connection has been established and data has recently been
//     received.
//   * dataTimeout: the connection is still active, but no data has been
//     received for more than a given time interval.
//   * connectionError: the connection could either not been established or was
//     lost after it had been established.
//     In this state, errorType indicates the type of error, if any, and
//     errorMessage may contain an error message string.
//
// State diagram:
//
//      .--------.   Close (user request) .---.
//      | Closed |<-----------------------| * |
//      '--------'                        '---'
//           | Open (user request)
//           V
//    .--------------. Error (type=failed) .---------------.
//    | Connecting   |-------------------->| Error         |
//    |--------------|                     |---------------|
//    | errorType    |             Timeout | errorType     |
//    | errorMessage |<--------------------| errorMessage  |
//    '--------------'                     | timeRemaining |
//           | Success                     '---------------'
//           V                                     ^
//      .---------. Error (type=lost)              |
//      | No data |--------------------------------|
//      '---------'                                |
//           | Data received                       |
//           V                                     |
//      .---------. Error (type=lost)              |
//      | Data OK |--------------------------------|
//      '---------'                                |
// Timeout |   ^                                   |
//         V   | Data received                     |
//    .--------------. Error (type=lost)           |
//    | Data timeout |-----------------------------'
//    '--------------'



// ******************
// ** Construction **
// ******************

ManagedDataStream::ManagedDataStream (QObject *parent): QObject (parent),
	_stream (NULL), _streamOwned (false), _open (false) //, _dataState (DataState::none)
	, _state (State::closed)
{
	// Create the timers. The timers will be deleted automatically by Qt.
	_dataTimer      = new QTimer (this);
	_reconnectTimer = new QTimer (this);

    // Setup the timers
    _dataTimer     ->setInterval (2000); _dataTimer     ->setSingleShot (true);
    _reconnectTimer->setInterval (2000); _reconnectTimer->setSingleShot (true);

    // Connect the timers' signals
    connect (_dataTimer     , SIGNAL (timeout ()), this, SLOT (dataTimer_timeout      ()));
    connect (_reconnectTimer, SIGNAL (timeout ()), this, SLOT (reconnectTimer_timeout ()));
}

ManagedDataStream::~ManagedDataStream ()
{
	// FIXME can this interfere with the stream being deleted by its parent?
	if (_streamOwned)
		delete _stream;
}


// ***********
// ** State **
// ***********

void ManagedDataStream::setState (ManagedDataStream::State::Type state)
{
	bool changed = (_state != state);

	// FIXME consistency checks
	_state=state;

	if (changed)
		emit stateChanged (_state);
}

ManagedDataStream::State::Type ManagedDataStream::getState () const
{
	return _state;
}

QString ManagedDataStream::stateText (ManagedDataStream::State::Type state)
{
	switch (state)
	{
		case State::closed:  return "closed";
		case State::opening: return "opening";
		case State::open:    return "open";
		case State::ok:      return "ok";
		case State::timeout: return "timeout";
		case State::error:   return "error";
		// No default
	}

	return "?";
}



// ****************************
// ** Underlying data stream **
// ****************************

void ManagedDataStream::setDataStream (DataStream *stream, bool streamOwned)
{
	// FIXME need to update the state!

	// If the stream is already current, stop
	if (_stream==stream)
		return;

	// Disconnect the old stream's signals
	if (_stream)
		_stream->disconnect (this);

	// Delete the old stream if we owned it
	if (_streamOwned)
		delete _stream;

	// Store the new stream
	_stream=stream;
	_streamOwned=streamOwned;

	if (_stream)
	{
		// Connect the new stream's signals
		connect (_stream, SIGNAL (stateChanged ()), this, SLOT (stream_stateChanged ()));
		connect (_stream, SIGNAL (dataReceived (QByteArray)), this, SLOT (stream_dataReceived (QByteArray)));

		// We changed the underlying data stream. We must now do two things:
		// get the data stream into the proper state, and update our own state
		// to match the data stream's state.


		// Update our own state (synthesize a stream state change event)
		stream_stateChanged ();

		// Update the new stream's state. If the stream is already in the
		// correct state, nothing will happen.
		if (_open)
			_stream->open ();
		else
			_stream->close ();
	}
}

void ManagedDataStream::clearDataStream ()
{
	setDataStream (NULL, false);
}

DataStream *ManagedDataStream::getDataStream () const
{
	return _stream;
}


// *********************
// ** Opening/closing **
// *********************

void ManagedDataStream::open ()
{
	_open=true;

	if (_stream)
	{
		// Open the stream. When the connection succeeds or fails (now or later),
		// we will receive a signal. This may cause timer to be started.
		_stream->open ();
	}
}

void ManagedDataStream::close ()
{
	_open=false;
	setState (State::closed);

	// Stop the timers. Note that a timer event may still be in the event queue,
	// so timer slots may be invoked even when the connection is closed.
	_dataTimer     ->stop ();
	_reconnectTimer->stop ();

	if (_stream)
	{
		// Close the stream
		_stream->close ();
	}
}

void ManagedDataStream::setOpen (bool o)
{
	if (o)
		open ();
	else
		close ();
}

bool ManagedDataStream::isOpen () const
{
	return _open;
}


// ************
// ** Timers **
// ************

// When closing the stream, the timers are stopped. Still, it is possible to
// receive a timer event while the stream is closed, in the following case:
//   * timer expires, timer event is enqueued
//   * stream is closed
//   * timer event is received
// Therefore, it is necessary to check if the state is open in the timer slots.

void ManagedDataStream::reconnectTimer_timeout ()
{
	qDebug () << "ManagedDataStream: reconnect timeout";

	// Ignore if the stream is not open (see above)
	if (!_open)
		return;

	if (_stream)
	{
		_stream->open ();
	}
}

/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void ManagedDataStream::dataTimer_timeout ()
{
	qDebug () << "ManagedDataStream: data timeout";

	// Ignore if the stream is not open (see above)
	if (!_open)
		return;

	setState (State::timeout);

	if (_stream)
	{

	}
}


// ************
// ** Stream **
// ************

void ManagedDataStream::stream_stateChanged ()
{
	DataStream::State state=_stream->getState ();

	qDebug () << "ManagedDataStream: stream state changed to" <<
		DataStream::stateText (state);

	switch (state)
	{
		case DataStream::closedState:
			if (_open)
			{
				_dataTimer->stop ();
				_reconnectTimer->start ();
				setState (State::error);
			}
			break;
		case DataStream::openingState:
			setState (State::opening);
			break;
		case DataStream::openState:
			setState (State::open);
			break;
		// no default
	}
}

void ManagedDataStream::stream_dataReceived (QByteArray data)
{
	setState (State::ok);
	emit dataReceived (data);

	// Start or restart the data timer
	_dataTimer->start ();
}
