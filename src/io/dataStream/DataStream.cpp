#include "src/io/dataStream/DataStream.h"

#include <QDebug>
#include <QTimer>


// The meaning of the individual states:
//   * streamClosed: the user has not requested the stream to be opened. If the
//     user has requested the stream to be opened, but the stream is not
//     actually connected, a different state is used (see below). All other
//     states imply that the user has requested the stream to be opened.
//   * streamConnecting: the connection is currently being established. This
//     may be the initial connect after opening the stream, or a reconnect after
//     a connection fails or is lost (see below).
//     In this state, errorType indicates the type of error (connection failed
//     or lost), if any, and errorMessage may contain an error message string.
//   * streamNoData: the connection has been established, but no data has been
//     received yet. This state does not indicate an error, it is always reached
//     after connecting successfully. If there is a data timeout after some data
//     has been received, streamDataTimeout is used (see below).
//   * streamDataOk: the connection has been established and data has recently
//     been received.
//   * streamDataTimeout: the connection is still active, but no data has been
//     received for more than a given time interval.
//   * connectionError: the connection could either not been established or was
//     lost after it had been established.
//     In this state, errorType indicates the type of error, if any, and
//     errorMessage may contain an error message string.
//
// The meaning of the ErrorType values:
//   * noError: no error occurred; in the streamConnecting state, this means
//     the initial connect after opening the stream
//   * connectionFailedError: an error occurred while the connection was being
//     opened
//   * connectionLostError: an error occurred while the connection was open

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

DataStream::DataStream ()
{
	// Initialize the state to closed
	_state.streamState=streamClosed;
	_state.timeRemaining=0;

    // Create the timers. The timers will be deleted automatically by their
	// parent, this.
    dataTimer      = new QTimer (this);
    reconnectTimer = new QTimer (this);

    // Setup the timers
    dataTimer     ->setInterval (2000); dataTimer     ->setSingleShot (true);
    reconnectTimer->setInterval (2000); reconnectTimer->setSingleShot (true);

    // Connect the timers' signals
    connect (dataTimer     , SIGNAL (timeout ()), this, SLOT (dataTimerTimeout      ()));
    connect (reconnectTimer, SIGNAL (timeout ()), this, SLOT (reconnectTimerTimeout ()));

}

DataStream::~DataStream ()
{
}


// **********************
// ** Public interface **
// **********************

DataStream::State DataStream::state () const
{
	return _state;
}

void DataStream::open ()
{
	// If the stream is already open, there's nothing to do.
	if (_state.isOpen ())
		return;

	// Update the state
	setState (streamConnecting);

	// Open the connection. When the connection succeeds (now or later),
	// connectionOpened will be called. When the connection fails (now or later)
	// or is closed (after it has been opened), connectionClosed will be called.
	openConnection ();
}

void DataStream::close ()
{
	// If the stream is already closed, there's nothing to do.
	if (!_state.isOpen ())
		return;

	// Update the state
	setState (streamClosed);

	// Stop the timers. Note that a timer event may still be in the event queue,
	// so timer slots may be invoked even when the connection is closed.
	dataTimer     ->stop ();
	reconnectTimer->stop ();

	// Close the connection
	closeConnection ();
}

void DataStream::setOpen (bool o)
{
	if (o)
		open ();
	else
		close ();
}


// ******************************
// ** Implementation interface **
// ******************************

void DataStream::connectionOpened ()
{
	setState (streamNoData);
	dataTimer->start ();
}

void DataStream::connectionClosed (const QString &message)
{
	// Stop the data timer. Note that a timer event may still be in the event
	// queue, so the timer slot may be invoked even when the connection is
	// closed.
	dataTimer->stop ();

	// Start the reconnect timer
	reconnectTimer->start ();

	// If the stream is closed or we're already in an error state, ignore the
	// event.
	if (_state.streamState==streamConnecting)
		setState (streamConnectionError, 0, connectionFailedError, message);
	else if (_state.isConnected ())
		setState (streamConnectionError, 0, connectionLostError, message);
}

/**
 * Closes and reopens the connection if it is open
 *
 * Call this method from a subclass when the connection parameters changed and
 * the connection may have to be reopened. Don't call it if the parameter values
 * were set, but did not actually change.
 */
void DataStream::parametersChanged ()
{
	if (_state.isOpen ())
	{
		close ();
		open ();
	}
}

void DataStream::dataReceived (const QByteArray &data)
{
	// Start or restart the data timer
	setState (streamDataOk);
	dataTimer->start ();

	// Process the data
	// Example:
	//   f o o \n b a r \n b
	//   0 1 2 3  4 5 6 7  8
	buffer.append (QString::fromUtf8 (data));
	int pos=0;
	while ((pos=buffer.indexOf ("\n"))>=0)
	{
		QString line=buffer.left (pos).trimmed ();
		buffer.remove (0, pos+1);
		emit lineReceived (line);
	}

}




// ******************
// ** Timer events **
// ******************

// When closing the stream, the timers are stopped. Still, it is possible to
// receive a timer event while the stream is closed, in the following case:
//   * timer expires, timer event is enqueued
//   * stream is closed
//   * timer event is received
// Therefore, it is necessary to check if the state is open in the timer slots.

void DataStream::reconnectTimerTimeout ()
{
	// Don't reconnect if the stream is not open (see above)
	if (_state.streamState == streamConnectionError)
	{
		// Keep the error state and message
		setState (streamConnecting, 0, _state.errorType, _state.errorMessage);
		openConnection ();
	}
}

/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void DataStream::dataTimerTimeout ()
{
	if (_state.streamState==streamDataOk || _state.streamState==streamNoData)
		setState (streamDataTimeout);
}


// ***********
// ** State **
// ***********

void DataStream::setState (StreamState streamState, int timeRemaining, ErrorType errorType, const QString &errorMessage)
{
	State newState;
	newState.streamState   = streamState;
	newState.timeRemaining = timeRemaining;
	newState.errorType     = errorType;
	newState.errorMessage  = errorMessage;

	if (newState!=_state)
	{
		_state = newState;
		emit stateChanged (_state);
	}
}


// ***********
// ** State **
// ***********

bool DataStream::State::operator== (const DataStream::State &other) const
{
	if (streamState   != other.streamState  ) return false;
	if (timeRemaining != other.timeRemaining) return false;
	if (errorType     != other.errorType    ) return false;
	if (errorMessage  != other.errorMessage ) return false;
	return true;
}

bool DataStream::State::operator!= (const DataStream::State &other) const
{
	return !(operator== (other));
}
