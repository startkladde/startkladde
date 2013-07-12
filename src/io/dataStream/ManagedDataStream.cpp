#include "ManagedDataStream.h"

#include <QDebug>
#include <QTimer>

#include "src/io/dataStream/BackgroundDataStream.h"

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
	_backgroundStream (NULL), _open (false), _state (State::closed)
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
	// _backgroundStream will be deleted automatically
	// _dataTimer will be deleted automatically
	// _reconnectTimer will be deleted automatically
}


// ***********
// ** State **
// ***********

void ManagedDataStream::goToState (ManagedDataStream::State::Type state)
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
	// If the stream is already current, stop
	if (_backgroundStream && _backgroundStream->getStream ()==stream)
		return;

	// Delete the old background stream. This will stop the thread, disconnect
	// its signals and, if the old underlying stream was owned, delete it.
	delete _backgroundStream;
	_backgroundStream=NULL;

	// If the stream is being set to NULL, there's nothing else to do.
	if (stream)
	{
		// Create a new background stream for the new stream
		_backgroundStream=new BackgroundDataStream (this, stream, streamOwned);

		// Connect the new background stream's signals
		connect (_backgroundStream, SIGNAL (stateChanged        (DataStream::State)),
		         this             , SLOT   (stream_stateChanged (DataStream::State)));
		connect (_backgroundStream, SIGNAL (dataReceived        (QByteArray)),
		         this             , SLOT   (stream_dataReceived (QByteArray)));

		// We changed the underlying data stream. We must now do two things:
		// get the data stream into the proper state, and update our own state
		// to match the data stream's state.

		// Update our own state (synthesize a stream state change event)
		stream_stateChanged (_backgroundStream->getState ());

		// Update the new stream's state. If the stream is already in the
		// correct state, nothing will happen.
		if (_open)
			_backgroundStream->open ();
		else
			_backgroundStream->close ();
	}
}

void ManagedDataStream::clearDataStream ()
{
	setDataStream (NULL, false);
}

// TODO we should remove this if possible, because we're not supposed to access
// the underlying data stream which now lives on a background thread.
DataStream *ManagedDataStream::getDataStream () const
{
	if (_backgroundStream)
		return _backgroundStream->getStream ();
	else
		return NULL;
}


// *********************
// ** Opening/closing **
// *********************

void ManagedDataStream::open ()
{
	_open=true;

	if (_backgroundStream)
	{
		// Open the stream. When the connection succeeds or fails (now or later),
		// we will receive a signal. This may cause timer to be started.
		_backgroundStream->open ();
	}
}

void ManagedDataStream::close ()
{
	_open=false;
	goToState (State::closed);

	// Stop the timers. Note that a timer event may still be in the event queue,
	// so timer slots may be invoked even when the connection is closed.
	_dataTimer     ->stop ();
	_reconnectTimer->stop ();

	if (_backgroundStream)
	{
		// Close the stream
		_backgroundStream->close ();
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

	if (_backgroundStream)
	{
		_backgroundStream->open ();
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

	goToState (State::timeout);

	if (_backgroundStream)
	{

	}
}


// ************
// ** Stream **
// ************

void ManagedDataStream::stream_stateChanged (DataStream::State state)
{
	qDebug () << "ManagedDataStream: stream state changed to" <<
		DataStream::stateText (state);

	switch (state)
	{
		case DataStream::closedState:
			if (_open)
			{
				_dataTimer->stop ();
				_reconnectTimer->start ();
				goToState (State::error);
			}
			break;
		case DataStream::openingState:
			goToState (State::opening);
			break;
		case DataStream::openState:
			goToState (State::open);
			break;
		// no default
	}
}

void ManagedDataStream::stream_dataReceived (QByteArray data)
{
	goToState (State::ok);
	emit dataReceived (data);

	// Start or restart the data timer
	_dataTimer->start ();
}
