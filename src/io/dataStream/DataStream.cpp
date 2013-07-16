#include "DataStream.h"

#include <QDebug>

#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

/**
 * Creates a new DataStream instance with the specified Qt parent
 */
DataStream::DataStream (QObject *parent): QObject (parent),
	_state (closedState)
{
	// The stream is initially in the closed state.
}

DataStream::~DataStream ()
{
}


// ********************
// ** User interface **
// ********************

/**
 * Opens the stream.
 *
 * If the stream is already opening or open, nothing happens. Otherwise, the
 * stream will go to the `opening` state. Depending on the implementation, it
 * may go to the `open` (on success) or `closed` (on failure) state immediately
 * before this method returns, or this method may return and the stream will go
 * to the `open` or `closed` state later.
 *
 * After calling this method, the state of the stream can be `opening` (if
 * opening is delayed, e. g. waiting for the remote side to accept the
 * connection), `open` (if the stream opened immediately) or `closed` (if the
 * connection failed immediately).
 */
void DataStream::open ()
{
	// If the stream is already open or opening, there's nothing to do.
	if (_state!=closedState)
		return;

	// Update the state
	goToState (openingState);

	// Open the stream. When the operation succeeds (now or later),
	// streamOpenSuccess will be called. When the operation fails (now or
	// later), streamOpenFailure will be called. When the error is closed due to
	// an error (after it has been opened), streamError will be called.
	openStream ();
}

/**
 * Closes the stream.
 *
 * If the stream is already closed, nothing happens. Otherwise, it will go to
 * the `closed` state before this method returns.
 */
void DataStream::close ()
{
	// If the stream is already closed, there's nothing to do.
	if (_state==closedState)
		return;

	// Update the state
	goToState (closedState);

	// Close the stream
	closeStream ();
}

/**
 * Calls either `open` (if the parameter is `true`) or `close` (otherwise).
 *
 * This method (slot) can be useful to connect to a signal of, e. g., a QAction
 * that controls opening of the stream.
 */
void DataStream::setOpen (bool o)
{
	if (o)
		open ();
	else
		close ();
}

/**
 * Returns the current state of the data stream.
 */
DataStream::State DataStream::getState () const
{
	return _state;
}

// FIXME now we have to make it thread-safe.
QString DataStream::getErrorMessage () const
{
	return _errorMessage;
}


// ***********************
// ** Private interface **
// ***********************

/**
 * Goes to the specified state.
 *
 * This method stores the specified state and emits a stateChanged signal.
 */
void DataStream::goToState (DataStream::State state)
{
	_state=state;
	emit stateChanged (state);
}


// ******************************
// ** Implementation interface **
// ******************************

/**
 * Called by implementations whenever the stream is successfully opened.
 */
void DataStream::streamOpened ()
{
	goToState (openState);
}

/**
 * Called by implementations whenever the stream fails to open or experiences
 * a fatal error.
 *
 * Implementations should make sure that the underlying mechanism is closed and
 * ready to be re-opened before calling this method.
 */
void DataStream::streamError (const QString &errorMessage)
{
	//qDebug () << "DataStream error:" << errorMessage;
	_errorMessage=errorMessage;
	goToState (closedState);
}

/**
 * Called by implementations when data is received from the stream.
 */
void DataStream::streamDataReceived (const QByteArray &data)
{
	emit dataReceived (data);
}

/**
 * Can be called by implementations when it seems likely that the connection can
 * now be established when it couldn't before; e. g. when the required hardware
 * is plugged in.
 *
 * The corresponding signal may, for example, be used to initiate a reconnect
 * the connection failed.
 *
 * Implementations are not required to support this mechanism.
 */
void DataStream::streamConnectionBecameAvailable ()
{
	emit connectionBecameAvailable ();
}


// ***********
// ** State **
// ***********

/**
 * Returns a readable representation of the state.
 *
 * This method can be useful for logging, but the return value should probably
 * not be shown to the user, since the text is not localized and may change in
 * the future.
 */
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
