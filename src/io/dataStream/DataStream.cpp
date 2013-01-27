#include "src/io/dataStream/DataStream.h"

#include <QDebug>
#include <QTimer>


// ******************
// ** Construction **
// ******************

DataStream::DataStream ()
{
	// Initialize the state to closed. The other parameters are irrelevant.
	_state.open=false;

    // Create the timers. The timers will be deleted automatically by their
	// parent, this.
    dataTimer      = new QTimer (this);
    reconnectTimer = new QTimer (this);

    // Setup the timers
    dataTimer     ->setInterval (2000); dataTimer     ->setSingleShot (true);
    reconnectTimer->setInterval (1000); reconnectTimer->setSingleShot (true);

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
	if (_state.open)
		return;

	// Update the state
	_state.open=true;
	_state.streamState=streamConnecting;

	// Open the connection. When the connection succeeds (now or later),
	// connectionOpened will be called. When the connection fails (now or later)
	// or is closed (after it has been opened), connectionClosed will be called.
	openConnection ();
}

void DataStream::close ()
{
	// If the stream is already closed, there's nothing to do.
	if (!_state.open)
		return;

	// Update the state
	_state.open=false;

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
	_state.streamState=streamConnected;
	_state.dataState=dataNone;

	dataTimer->start ();
}

void DataStream::connectionClosed ()
{
	// Stop the data timer. Note that a timer event may still be in the event
	// queue, so the timer slot may be invoked even when the connection is
	// closed.
	dataTimer     ->stop ();

	// Start the reconnect timer
	reconnectTimer->start ();

	if (_state.streamState==streamConnecting)
	{
		_state.streamState=streamConnectionFailed;
	}
	else if (_state.streamState==streamConnected)
	{
		_state.streamState=streamConnectionLost;
	}
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
	if (_state.open)
	{
		close ();
		open ();
	}
}

void DataStream::dataReceived (const QByteArray &data)
{
	// Start or restart the data timer
	dataTimer->start ();
	_state.dataState=dataOk;

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
	if (!_state.open)
		return;

	openConnection ();
}

/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void DataStream::dataTimerTimeout ()
{
	// Don't signal a timeout if the stream is not open (see above) or not
	// connected
	if (!_state.open || _state.streamState!=streamConnected)
		return;

	_state.dataState=dataTimeout;
}
