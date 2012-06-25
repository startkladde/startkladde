#ifndef TCPDATASTREAM_H_
#define TCPDATASTREAM_H_

#include <QObject>
#include <QAbstractSocket>

class QTcpSocket;
class QTimer;

class TcpDataStream: public QObject
{
		Q_OBJECT

	public:
		enum ConnectionState
		{
			notConnected,
			connecting,
			connected,
		};

		class State
		{
			public:
				State ();

				bool isOpen          () { return open;            }
				TcpDataStream::ConnectionState getConnectionState ()
				                        { return connectionState; }
				bool getDataReceived () { return dataReceived;    }
				bool getDataTimeout  () { return dataTimeout;     }

				bool setOpen (bool open);
				bool setConnectionState (TcpDataStream::ConnectionState connectionState);
				bool setDataReceived (bool dataReceived);
				bool setDataTimeout (bool dataTimeout);

			private:
				// Whether the user has requested a connection to be established
				bool open;
				TcpDataStream::ConnectionState connectionState;
				bool dataReceived;
				bool dataTimeout;
		};

		TcpDataStream ();
		virtual ~TcpDataStream ();

		void open ();
		void close ();

		State getState ();

	signals:
		void stateChanged (TcpDataStream::State state);
		void lineReceived (const QString &line);

	protected:
		void openSocket ();
		void closeSocket ();

	private:
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
