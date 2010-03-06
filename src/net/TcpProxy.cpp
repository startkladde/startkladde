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


TcpProxy::TcpProxy ():
	server (NULL), serverSocket (NULL), clientSocket (NULL)
{
	std::cout << "Creating a TcpProxy on thread " << QThread::currentThreadId () << std::endl;

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
	std::cout << "Open server in thread " << QThread::currentThreadId () << std::endl;

	// Store the connection data
	this->serverHost=serverHost;
	this->serverPort=serverPort;

	delete server;
	server=new QTcpServer (this);

	connect (server, SIGNAL (newConnection ()), this, SLOT (newConnection ()));

	if (server->isListening ()) server->close ();

	if (server->listen (QHostAddress::LocalHost, 0))
		std::cout << "Ok, listening on port " << server->serverPort () << std::endl;
	else
		std::cout << "Listen failed" << std::endl;

	return server->serverPort ();
}

/**
 * Closes the connection, if there is any
 */
void TcpProxy::slot_close ()
{
	std::cout << "Close connection in thread " << QThread::currentThreadId () << std::endl;
	closeClientSocket ();
	closeServerSocket ();
}

void TcpProxy::closeClientSocket ()
{
	std::cout << "close client socket" << std::endl;
	if (!clientSocket) return;

	clientSocket->disconnect(); // signals
	clientSocket->close ();
	clientSocket->deleteLater ();
	clientSocket=NULL;
}

void TcpProxy::closeServerSocket ()
{
	std::cout << "close server socket" << std::endl;
	if (!serverSocket) return;

	serverSocket->disconnect ();
	serverSocket->close ();
	serverSocket->deleteLater ();
	serverSocket=NULL;
}

void TcpProxy::newConnection ()
{
	std::cout << "new connection on thread " << QThread::currentThreadId () << std::endl;

	std::cout << utf8 ("connecting to %1:%2").arg (serverHost).arg (serverPort) << std::endl;

	delete serverSocket;
	serverSocket=new QTcpSocket (this);
	serverSocket->connectToHost (serverHost, serverPort);

	std::cout << "Returned from connect" << std::endl;

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
//	std::cout << "write to server..." << std::flush;
	if (serverSocket) serverSocket->write (clientSocket->readAll ());
//	std::cout << "done" << std::endl;
}

void TcpProxy::serverRead ()
{
//	std::cout << "write to client..." << std::flush;
	if (clientSocket) clientSocket->write (serverSocket->readAll ());
//	std::cout << "done" << std::endl;
}

void TcpProxy::clientClosed ()
{
//	std::cout << "client closed" << std::endl;
	slot_close ();
}

void TcpProxy::serverClosed ()
{
//	std::cout << "server closed" << std::endl;
	slot_close ();
}

void TcpProxy::clientError ()
{
//	std::cout << "client error" << std::endl;
	slot_close ();
}

void TcpProxy::serverError ()
{
//	std::cout << "server error" << std::endl;
	slot_close ();
}
