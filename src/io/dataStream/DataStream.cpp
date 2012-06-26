#include "src/io/dataStream/DataStream.h"

#include <QDebug>
#include <QTimer>

DataStream::DataStream ()
{
    // Create the timers
    dataTimer      = new QTimer (this);
    reconnectTimer = new QTimer (this);

    // Setup the timers
    dataTimer     ->setInterval (2000); dataTimer     ->setSingleShot (true);
    reconnectTimer->setInterval (5000); reconnectTimer->setSingleShot (true);

    // Connect the timers' signals
    connect (dataTimer     , SIGNAL (timeout ()), this, SLOT (dataTimerTimeout      ()));
    connect (reconnectTimer, SIGNAL (timeout ()), this, SLOT (reconnectTimerTimeout ()));
}

DataStream::~DataStream ()
{
    delete dataTimer;
    delete reconnectTimer;
}


// ****************
// ** Connection **
// ****************

DataStream::State DataStream::getState ()
{
	return state;
}


void DataStream::open ()
{
	// Nothing to do if the stream is already open
	if (!state.isOpen ())
	{
		//qDebug () << "DataStream: open";

		state.setOpen (true);
		emit stateChanged (state);

		openImplementation ();
	}
}

void DataStream::close ()
{
	// Nothing to do if the stream is already closed
	if (state.isOpen ())
	{
		//qDebug () << "DataStream: close";

		state.setOpen (false);
		emit stateChanged (state);

		dataTimer     ->stop ();
		reconnectTimer->stop ();
		closeImplementation ();
	}
}


// ************
// ** Status **
// ************

void DataStream::dataReceived ()
{
	if (state.setDataReceived (true) || state.setDataTimeout (false))
		emit stateChanged (state);

	// Start or restart the data timer
	dataTimer->start ();
}

void DataStream::connectionOpening ()
{
	// Seems like we're currently connecting
	state.setConnectionState (connecting);
	emit stateChanged (state);
}


void DataStream::connectionEstablished ()
{
	state.setConnectionState (connected);
	state.setDataReceived (false);
	state.setDataTimeout (false);
	emit stateChanged (state);

	dataTimer->start ();
}

void DataStream::connectionFailed ()
{
	dataTimer     ->stop ();
	reconnectTimer->start ();

	state.setConnectionState (notConnected);
	emit stateChanged (state);
}

void DataStream::connectionLost ()
{
	dataTimer     ->stop ();
	reconnectTimer->start ();

	state.setConnectionState (notConnected);
	emit stateChanged (state);
}


// ************
// ** Timers **
// ************

// When closing the stream, the timers are stopped. Still, it is possible to
// receive a timer event while the stream is closed, in the following case:
//   * timer expires, timer even is enqueued
//   * stream is closed
//   * timer event is received
// Therefore, it is necessary to check if the state is open in the timer slots.

void DataStream::reconnectTimerTimeout ()
{
	if (state.isOpen ())
	{
		//qDebug () << "DataStream: automatic reconnect";
		openImplementation ();
	}
}


/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void DataStream::dataTimerTimeout ()
{
	if (state.isOpen ())
	{
		//qDebug () << "DataStream: data timeout";

		if (state.setDataTimeout (true))
			emit stateChanged (state);
	}
}


// *******************************
// ** DataStream::State methods **
// *******************************

DataStream::State::State ():
	open (false), connectionState (notConnected),
	dataReceived (false), dataTimeout (false)
{
}

bool DataStream::State::setOpen (bool open)
{
	bool result=(open!=this->open);
	this->open=open;
	return result;
}

bool DataStream::State::setConnectionState (DataStream::ConnectionState connectionState)
{
	bool result=(connectionState!=this->connectionState);
	this->connectionState=connectionState;
	return result;
}

bool DataStream::State::setDataReceived (bool dataReceived)
{
	bool result=(dataReceived!=this->dataReceived);
	this->dataReceived=dataReceived;
	return result;
}

bool DataStream::State::setDataTimeout (bool dataTimeout)
{
	bool result=(dataTimeout!=this->dataTimeout);
	this->dataTimeout=dataTimeout;
	return result;
}
