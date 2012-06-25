#ifndef TCPDATASTREAM_H_
#define TCPDATASTREAM_H_

#include <QAbstractSocket>

#include "src/io/dataStream/DataStream.h"

class QTcpSocket;

class TcpDataStream: public DataStream
{
		Q_OBJECT

	public:
		TcpDataStream ();
		virtual ~TcpDataStream ();

		void setHost (const QString &host);
		void setPort (const uint16_t port);
		void setTarget (const QString &host, const uint16_t port);

	protected:
		void openImplementation ();
		void closeImplementation ();

	private:
		QString host;
		uint16_t port;

	    QTcpSocket *socket;

	private slots:
    	void socketDataReceived ();
		void socketStateChanged (QAbstractSocket::SocketState socketState);
		void socketError (QAbstractSocket::SocketError error);
};

#endif
