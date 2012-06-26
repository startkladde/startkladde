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
	connect (socket, SIGNAL (readyRead    ()                            ), this, SLOT (socketDataReceived ()                            ));
    connect (socket, SIGNAL (error        (QAbstractSocket::SocketError)), this, SLOT (socketError        (QAbstractSocket::SocketError)));
    connect (socket, SIGNAL (stateChanged (QAbstractSocket::SocketState)), this, SLOT (socketStateChanged (QAbstractSocket::SocketState)));

}

TcpDataStream::~TcpDataStream ()
{
	// Close the connection (nothing will happen if it is already closed)
	close ();

    delete socket;
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


// ************************
// ** DataStream methods **
// ************************

void TcpDataStream::openImplementation ()
{
	// Nothing to do if the connection is already open or currently opening
	if (socket->state ()==QAbstractSocket::UnconnectedState)
	{
		socket->connectToHost (host, port, QIODevice::ReadOnly);
		connectionOpening ();
	}
}

void TcpDataStream::closeImplementation ()
{
	// We can do this even if the connection is not open
	socket->abort ();
}


// **********
// ** Data **
// **********

/**
 * Called when data is received from the socket
 *
 * Updates the connection state, resets the data timer, and emits lines.
 */
void TcpDataStream::socketDataReceived ()
{
	dataReceived ();

	// Read lines from the socket and emit them
	while (socket->canReadLine ())
	{
		QString line = socket->readLine ().trimmed ();
		emit lineReceived (line);
	}
}


// **********************
// ** Connection state **
// **********************

void TcpDataStream::socketStateChanged (QAbstractSocket::SocketState socketState)
{
	//qDebug () << "TcpDataStream: socket state changed to" << socketState;

	if (socketState == QAbstractSocket::ConnectedState)
	{
		connectionEstablished ();
	}
}

void TcpDataStream::socketError (QAbstractSocket::SocketError error)
{
	Q_UNUSED (error);

	//qDebug () << "TcpDataStream: socket error:" << error << "in socket state" << socket->state () ;

	if (socket->state ()==QAbstractSocket::ConnectedState)
		// We were connected
		connectionLost ();
	else
		connectionFailed ();

	// Make sure that the socket is really closed, and close it immediately,
	// without waiting for any buffers
	socket->abort ();
}
