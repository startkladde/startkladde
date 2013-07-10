#include "DataStream.h"

#include "src/i18n/notr.h"


// ******************
// ** Construction **
// ******************

DataStream::DataStream (QObject *parent): QObject (parent),
	_state (closedState)
{
}

DataStream::~DataStream ()
{
}


// ********************
// ** User interface **
// ********************

void DataStream::open ()
{
	// If the stream is already open or opening, there's nothing to do.
	if (_state!=closedState)
		return;

	// Update the state
	setState (openingState);

	// Open the stream. When the operation succeeds (now or later),
	// streamOpenSuccess will be called. When the operation fails (now or
	// later), streamOpenFailure will be called. When the error is closed due to
	// an error (after it has been opened), streamError will be called.
	openStream ();
}

void DataStream::close ()
{
	// If the stream is already closed, there's nothing to do.
	if (_state==closedState)
		return;

	// Update the state
	setState (closedState);

	// Close the stream
	closeStream ();
}

void DataStream::setOpen (bool o)
{
	if (o)
		open ();
	else
		close ();
}

DataStream::State DataStream::getState ()
{
	return _state;
}


// ***********************
// ** Private interface **
// ***********************

void DataStream::setState (DataStream::State state)
{
	_state=state;
	emit stateChanged ();
}


// ******************************
// ** Implementation interface **
// ******************************

void DataStream::streamOpened ()
{
	setState (openState);
}

void DataStream::streamError ()
{
	setState (closedState);
}

void DataStream::streamDataReceived (const QByteArray &data)
{
	emit dataReceived (data);
}


// FIXME enable or remove
///**
// * Closes and reopens the connection if it is open
// *
// * Call this method from a subclass when the connection parameters changed and
// * the connection may have to be reopened. Don't call it if the parameter values
// * were set, but did not actually change.
// */
//void DataStream::parametersChanged ()
//{
//	if (_state.streamState!=streamClosed)
//	{
//		close ();
//		open ();
//	}
//}


// ***********
// ** State **
// ***********

QString DataStream::stateText (DataStream::State state)
{
	switch (state)
	{
		case closedState : return notr ("closed" );
		case openingState: return notr ("opening");
		case openState   : return notr ("open"   );
		// no default
	}

	return notr ("?");
}
