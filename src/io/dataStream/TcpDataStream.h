#ifndef TCPDATASTREAM_H_
#define TCPDATASTREAM_H_

#include <QAbstractSocket>

#include "src/io/dataStream/DataStream.h"

class QTcpSocket;
class QTimer;

class TcpDataStream: public DataStream
{
		Q_OBJECT

	public:
		TcpDataStream ();
		virtual ~TcpDataStream ();

		void setHost (const QString &host);
		void setPort (const uint16_t port);
		void setTarget (const QString &host, const uint16_t port);

		void open ();
		void close ();

		State getState ();

	signals:
		void stateChanged (DataStream::State state);
		void lineReceived (const QString &line);

	protected:
		void openSocket ();
		void closeSocket ();

	private:
		QString host;
		uint16_t port;

	    QTcpSocket* socket;
	    QTimer *dataTimer;
	    QTimer *reconnectTimer;

	    State state;

	private slots:
    	void dataReceived ();
		void socketStateChanged (QAbstractSocket::SocketState socketState);
		void socketError (QAbstractSocket::SocketError error);
		void dataTimerTimeout ();
		void reconnectTimerTimeout ();
};

#endif
