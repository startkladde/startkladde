/*
 * Improvements:
 *   - If the server port is closed, don't accept the client connection.
 *     However, the connection is accepted by QTcpServer
 *   - Make the error source (QAbstractSocket::SocketError socketError)
 *     available to the caller (and use it)
 *   - More sophisticated error handling: currently we just close the
 *     connection on any error (and let the auto-reconnect pick up)
 */
#include "TcpProxy.h"

#include <iostream>

#include "src/util/qString.h"
#include "src/concurrent/Returner.h"

//#define DEBUG(stuff) do { std::cout << stuff << std::endl; } while (0)
#define DEBUG(stuff)

TcpProxy::TcpProxy ():
	server (NULL), serverSocket (NULL), clientSocket (NULL)
{
	DEBUG ("Creating a TcpProxy on thread " << QThread::currentThreadId ());

	connect (this, SIGNAL (sig_open (Returner<quint16> *, QString, quint16)), this, SLOT (slot_open (Returner<quint16> *, QString, quint16)));
	connect (this, SIGNAL (sig_close ()), this, SLOT (slot_close ()));

	moveToThread (&proxyThread);
	proxyThread.start ();
}

TcpProxy::~TcpProxy ()
{
	proxyThread.quit ();

	std::cout << "Waiting for proxy thread to terminate...";
	std::cout.flush ();

	if (proxyThread.wait (1000))
		std::cout << "OK" << std::endl;
	else
		std::cout << "Timeout" << std::endl;

	// server, serverSocket and clientSocket will be deleted automatically
}

// ****************
// ** Properties **
// ****************

quint16 TcpProxy::getProxyPort ()
{
	synchronizedReturn (mutex, proxyPort);
}

// ***************
// ** Frontends **
// ***************

quint16 TcpProxy::open (const QString &serverHost, quint16 serverPort)
{
	Returner<quint16> returner;
	emit sig_open (&returner, serverHost, serverPort);
	return returner.returnedValue ();
}

void TcpProxy::close ()
{
	emit sig_close ();
}

// **************
// ** Backends **
// **************

/**
 * Opens the proxy server, but not the connection to the (real) server
 */
void TcpProxy::slot_open (Returner<quint16> *returner, QString serverHost, quint16 serverPort)
{
	returnOrException (returner, openImpl (serverHost, serverPort));
}

quint16 TcpProxy::openImpl (QString serverHost, quint16 serverPort)
{
	// TODO only if host and port matches
	if (server && server->isListening ()) return server->serverPort ();

	DEBUG ("Open server in thread " << QThread::currentThreadId ());

	// Store the connection data
	synchronized (mutex)
	{
		this->serverHost=serverHost;
		this->serverPort=serverPort;
	}

	delete server;
	server=new QTcpServer (this);

	connect (server, SIGNAL (newConnection ()), this, SLOT (newConnection ()));

	if (server->isListening ()) server->close ();

	if (server->listen (QHostAddress::LocalHost, 0))
	{
		synchronized (mutex) this->proxyPort=server->serverPort ();

		DEBUG ("Ok, listening on port " << server->serverPort ());
	}
	else
	{
		DEBUG ("Listen failed");
	}

	return server->serverPort ();
}

/**
 * Closes the connection, if there is any
 */
void TcpProxy::slot_close ()
{
	DEBUG ("Close connection in thread " << QThread::currentThreadId ());
	closeClientSocket ();
	closeServerSocket ();
}

void TcpProxy::closeClientSocket ()
{
	DEBUG ("close client socket");
	if (!clientSocket) return;

	if (serverSocket) clientSocket->write (serverSocket->read (serverSocket->bytesAvailable ()));
	clientSocket->flush ();

	clientSocket->disconnect(); // signals
	clientSocket->close ();
	clientSocket->deleteLater ();
	clientSocket=NULL;
}

void TcpProxy::closeServerSocket ()
{
	DEBUG ("close server socket");

	if (!serverSocket) return;

	if (clientSocket) serverSocket->write (clientSocket->read (clientSocket->bytesAvailable ()));
	serverSocket->flush ();

	serverSocket->disconnect ();
	serverSocket->close ();
	serverSocket->deleteLater ();
	serverSocket=NULL;
}

void TcpProxy::newConnection ()
{
	DEBUG ("new connection on thread " << QThread::currentThreadId ());
	DEBUG (utf8 ("connecting to %1:%2").arg (serverHost).arg (serverPort));

	delete serverSocket;
	serverSocket=new QTcpSocket (this);
	serverSocket->connectToHost (serverHost, serverPort);

	delete clientSocket;
	clientSocket=server->nextPendingConnection ();

	connect (serverSocket, SIGNAL (readyRead ()), this, SLOT (serverRead ()));
	connect (clientSocket, SIGNAL (readyRead ()), this, SLOT (clientRead ()));

	connect (serverSocket, SIGNAL (disconnected ()), this, SLOT (serverClosed ()));
	connect (clientSocket, SIGNAL (disconnected ()), this, SLOT (clientClosed ()));

	connect (serverSocket, SIGNAL (error (QAbstractSocket::SocketError)), this, SLOT (serverError ()));
	connect (clientSocket, SIGNAL (error (QAbstractSocket::SocketError)), this, SLOT (clientError ()));
}

void TcpProxy::clientRead ()
{
	DEBUG ("write to server...");

	if (serverSocket) serverSocket->write (clientSocket->readAll ());

	DEBUG ("done");
}

void TcpProxy::serverRead ()
{
	DEBUG ("write to client...");

	if (clientSocket) clientSocket->write (serverSocket->readAll ());

	DEBUG ("done");
}

void TcpProxy::clientClosed ()
{
	DEBUG ("client closed");
	slot_close ();
}

void TcpProxy::serverClosed ()
{
	DEBUG ("server closed");
	slot_close ();
}

void TcpProxy::clientError ()
{
	DEBUG ("client error");
	slot_close ();
}

void TcpProxy::serverError ()
{
	DEBUG ("server error");
	slot_close ();
}
