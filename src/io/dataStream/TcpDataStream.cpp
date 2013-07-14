#include "TcpDataStream.h"

#include <QTcpSocket>


// ******************
// ** Construction **
// ******************

TcpDataStream::TcpDataStream (QObject *parent): DataStream (parent),
	_port (0)
{
	// Create the socket and connect the required signals. _socket will be
	// deleted automatically by its parent (this).
	// FIXME don't create in constructor - allow calling open() from a different
	// thread than the constructor
	_socket=new QTcpSocket (this);
	connect (_socket, SIGNAL (readyRead           ()),
	         this   , SLOT   (socket_dataReceived ()));
    connect (_socket, SIGNAL (error        (QAbstractSocket::SocketError)),
             this   , SLOT   (socket_error (QAbstractSocket::SocketError)));
    connect (_socket, SIGNAL (stateChanged        (QAbstractSocket::SocketState)),
             this   , SLOT   (socket_stateChanged (QAbstractSocket::SocketState)));
}

TcpDataStream::~TcpDataStream ()
{
}


// ****************
// ** Properties **
// ****************

/**
 * Sets the target host and port.
 */
void TcpDataStream::setTarget (const QString &host, uint16_t port)
{
	_host=host;
	_port=port;
}


// ************************
// ** DataStream methods **
// ************************

void TcpDataStream::openStream ()
{
	if (_socket->state () != QAbstractSocket::UnconnectedState)
		_socket->abort ();

	_socket->connectToHost (_host, _port, QIODevice::ReadOnly);
}

void TcpDataStream::closeStream ()
{
	_socket->abort ();
}


// *******************
// ** Socket events **
// *******************

/**
 * Called when data is received from the socket
 */
void TcpDataStream::socket_dataReceived ()
{
	dataReceived (_socket->readAll ());
}

/**
 * Called when the socked state changes, e. g. when the connection is
 * established.
 */
void TcpDataStream::socket_stateChanged (QAbstractSocket::SocketState socketState)
{
	//qDebug () << "TcpDataStream: socket state changed to" << socketState;

	if (socketState == QAbstractSocket::ConnectedState)
		streamOpened ();
}

/**
 * Called when the socket connection fails (while opening) or is lost (while
 * open).
 */
void TcpDataStream::socket_error (QAbstractSocket::SocketError error)
{
	Q_UNUSED (error);

	//qDebug () << "TcpDataStream: socket error:" << error << "in socket state" << socket->state () ;

	// Make sure that the socket is closed
	_socket->abort ();
//	streamError (_socket->errorString ()); // FIXME do use error string
	streamError ();
}
