#ifndef TCPDATASTREAM_H_
#define TCPDATASTREAM_H_

#include <stdint.h>
#include <QAbstractSocket>

#include "src/io/dataStream/DataStream.h"

class QTcpSocket;

class TcpDataStream: public DataStream
{
		Q_OBJECT

	public:
		TcpDataStream ();
		virtual ~TcpDataStream ();

		void setTarget (const QString &host, uint16_t port);

	protected:
		void openImplementation ();
		void closeImplementation ();

	private:
	    QTcpSocket *_socket;

		QString _host;
		uint16_t _port;

	private slots:
    	void socketDataReceived ();
		void socketStateChanged (QAbstractSocket::SocketState socketState);
		void socketError (QAbstractSocket::SocketError error);
};

#endif
