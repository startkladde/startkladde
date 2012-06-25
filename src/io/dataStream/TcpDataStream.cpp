#include "TcpDataStream.h"

#include <QTcpSocket>
#include <QTimer>

/*
 * Implementation notes:
 *   - after updating state, emit stateChanged when the update method returned
 *     true
 */

// ******************
// ** Construction **
// ******************

TcpDataStream::TcpDataStream ()
{
	// Note that we don't send a signal for the initial connection state. This
	// is the constructor, so no connection can have been made at this point.

	// Create the socket and connect the required signals
	socket=new QTcpSocket (this);
	connect (socket, SIGNAL (readyRead    ()                            ), this, SLOT (dataReceived       ()                            ));
    connect (socket, SIGNAL (error        (QAbstractSocket::SocketError)), this, SLOT (socketError        (QAbstractSocket::SocketError)));
    connect (socket, SIGNAL (stateChanged (QAbstractSocket::SocketState)), this, SLOT (socketStateChanged (QAbstractSocket::SocketState)));

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

TcpDataStream::~TcpDataStream ()
{
	// Close the connection (nothing will happen if it is already closed)
	close ();

    delete socket;
    delete dataTimer;
    delete reconnectTimer;
}


// ****************
// ** Properties **
// ****************

void TcpDataStream::setHost (const QString &host)
{
	this->host=host;
}

void TcpDataStream::setPort (const uint16_t port)
{
	this->port=port;
}

void TcpDataStream::setTarget (const QString &host, const uint16_t port)
{
	this->host=host;
	this->port=port;
}

// ****************
// ** Connection **
// ****************

void TcpDataStream::openSocket ()
{
	// Nothing to do if the connection is already open or currently opening
	if (socket->state ()==QAbstractSocket::UnconnectedState)
		socket->connectToHost (host, port, QIODevice::ReadOnly);
}

void TcpDataStream::closeSocket ()
{
	// We can do this even if the connection is not open
	socket->abort ();
}

void TcpDataStream::open ()
{
	// Nothing to do if the stream is already open
	if (!state.isOpen ())
	{
		qDebug () << "TcpDataStream: open";
		if (state.setOpen (true))
			emit stateChanged (state);

		openSocket ();
	}
}

void TcpDataStream::close ()
{
	// Nothing to do if the stream is already closed
	if (state.isOpen ())
	{
		qDebug () << "TcpDataStream: close";

		if (state.setOpen (false))
			emit stateChanged (state);

		closeSocket ();
	}
}


// **********************
// ** Connection state **
// **********************

TcpDataStream::State TcpDataStream::getState ()
{
	return state;
}


void TcpDataStream::socketStateChanged (QAbstractSocket::SocketState socketState)
{
	// The socket state changed. Update the connection state accordingly.
	qDebug () << "TcpDataStream: socket state changed to" << socketState;

	if (socketState == QAbstractSocket::ConnectedState)
	{
		state.setConnectionState (connected);
		state.setDataReceived (false);
		state.setDataTimeout (false);
		emit stateChanged (state);

		dataTimer->start ();
	}
	else if (socketState == QAbstractSocket::UnconnectedState)
	{
		state.setConnectionState (notConnected);
		emit stateChanged (state);
	}
	else
	{
		// Seems like we're currently connecting
		state.setConnectionState (connecting);
		emit stateChanged (state);
	}
}


void TcpDataStream::socketError (QAbstractSocket::SocketError error)
{
	Q_UNUSED (error);

	qDebug () << "TcpDataStream: socket error: " << error;

	// Make sure that the socket is really closed
	socket->abort ();

	reconnectTimer->start ();
	// FIXME need this if we don't get state closed
	//dataTimer->stop ();
}

void TcpDataStream::reconnectTimerTimeout ()
{
	qDebug () << "TcpDataStream: automatic reconnect" << endl;

	if (state.isOpen ())
		openSocket ();
}


// **********
// ** Data **
// **********

/**
 * Called when data is received from the socket
 *
 * Updates the connection state, resets the data timer, and emits lines.
 */
void TcpDataStream::dataReceived ()
{
	if (state.setDataReceived (true) || state.setDataTimeout (false))
		emit stateChanged (state);

	// Start or restart the data timer
	dataTimer->start ();

	// Read lines from the socket and emit them
	while (socket->canReadLine ())
	{
		QString line = socket->readLine ().trimmed ();
		emit lineReceived (line);
	}
}

/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void TcpDataStream::dataTimerTimeout ()
{
	qDebug () << "TcpDataStream: data timeout" << endl;

	if (state.setDataTimeout (true))
		emit stateChanged (state);
}

