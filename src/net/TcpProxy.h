/*
 * TcpProxy.h
 *
 *  Created on: 06.03.2010
 *      Author: Martin Herrmann
 */

#ifndef TCPPROXY_H_
#define TCPPROXY_H_

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QThread>

template<class T> class Returner;

class TcpProxy: public QObject
{
	Q_OBJECT

	public:
		TcpProxy ();
		virtual ~TcpProxy ();

		quint16 open (const QString &serverHost, quint16 serverPort);
		void close ();

	signals:
		void sig_open (Returner<quint16> *returner, QString serverHost, quint16 serverPort);
		void sig_close ();

	protected slots:
		void slot_open (Returner<quint16> *returner, QString serverHost, quint16 serverPort);
		void slot_close ();

		quint16 openImpl (QString serverHost, quint16 serverPort);

		void newConnection ();

		void clientRead ();
		void serverRead ();

		void clientClosed ();
		void serverClosed ();

		void clientError ();
		void serverError ();

	protected:
		void closeServerSocket ();
		void closeClientSocket ();

	private:
		QTcpServer *server;
		QTcpSocket *serverSocket;
		QTcpSocket *clientSocket;

		QThread proxyThread;

		QString serverHost;
		quint16 serverPort;
};

#endif
