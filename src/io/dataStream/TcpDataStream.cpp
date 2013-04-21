#include "TcpDataStream.h"

#include <QTcpSocket>
#include <QTimer>

// ******************
// ** Construction **
// ******************

TcpDataStream::TcpDataStream (QObject *parent): DataStream (parent),
	_port (0)
{
	// Note that we don't send a signal for the initial connection state. This
	// is the constructor, so no connection can have been made at this point.

	// Create the socket and connect the required signals. _socket will be
	// deleted automatically by its parent (this).
	_socket=new QTcpSocket (this);
	connect (_socket, SIGNAL (readyRead    ()                            ), this, SLOT (socketDataReceived ()                            ));
    connect (_socket, SIGNAL (error        (QAbstractSocket::SocketError)), this, SLOT (socketError        (QAbstractSocket::SocketError)));
    connect (_socket, SIGNAL (stateChanged (QAbstractSocket::SocketState)), this, SLOT (socketStateChanged (QAbstractSocket::SocketState)));
}

TcpDataStream::~TcpDataStream ()
{
}


// ****************
// ** Properties **
// ****************

void TcpDataStream::setTarget (const QString &host, uint16_t port)
{
	bool changed=false;
	if (host!=_host) changed=true;
	if (port!=_port) changed=true;

	_host=host;
	_port=port;

	if (changed)
		parametersChanged ();
}


// ************************
// ** DataStream methods **
// ************************

void TcpDataStream::openConnection  ()
{
	if (_socket->state () != QAbstractSocket::UnconnectedState)
		_socket->abort ();

	_socket->connectToHost (_host, _port, QIODevice::ReadOnly);
}

void TcpDataStream::closeConnection ()
{
	_socket->abort ();
}


// *******************
// ** Socket events **
// *******************

/**
 * Called when data is received from the socket
 */
void TcpDataStream::socketDataReceived ()
{
	dataReceived (_socket->readAll ());
}


void TcpDataStream::socketStateChanged (QAbstractSocket::SocketState socketState)
{
	//qDebug () << "TcpDataStream: socket state changed to" << socketState;

	if (socketState == QAbstractSocket::ConnectedState)
		connectionOpened ();
}

/**
 * Invoked when the socket connection failed (while opening) or was lost (while
 * open).
 */
void TcpDataStream::socketError (QAbstractSocket::SocketError error)
{
	Q_UNUSED (error);

	//qDebug () << "TcpDataStream: socket error:" << error << "in socket state" << socket->state () ;

	// Make sure that the socket is closed
	_socket->abort ();
	connectionClosed (_socket->errorString ());
}
