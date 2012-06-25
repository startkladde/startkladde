#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QObject>

class QTimer;

class DataStream: public QObject
{
		Q_OBJECT

	public:
		enum ConnectionState
		{
			notConnected,
			connecting,
			connected,
		};

		/**
		 * Represents the state of a DataStream
		 */
		class State
		{
			public:
				State ();

				bool isOpen          () { return open;            }
				DataStream::ConnectionState getConnectionState ()
				                        { return connectionState; }
				bool getDataReceived () { return dataReceived;    }
				bool getDataTimeout  () { return dataTimeout;     }

				bool setOpen (bool open);
				bool setConnectionState (DataStream::ConnectionState connectionState);
				bool setDataReceived (bool dataReceived);
				bool setDataTimeout (bool dataTimeout);

			private:
				// Whether the user has requested a connection to be established
				bool open;
				DataStream::ConnectionState connectionState;
				bool dataReceived;
				bool dataTimeout;
		};


		DataStream ();
		virtual ~DataStream ();

		virtual void open ();
		virtual void close ();

		State getState ();

	signals:
		void stateChanged (DataStream::State state);
		void lineReceived (const QString &line);

	protected:
		virtual void openImplementation ()=0;
		virtual void closeImplementation ()=0;

		virtual void dataReceived ();
		virtual void connectionEstablished ();
		virtual void connectionLost ();
		virtual void connectionOpening ();
		virtual void connectionClosed ();

	private:
	    QTimer *dataTimer;
	    QTimer *reconnectTimer;

	    State state;

	private slots:
		void dataTimerTimeout ();
		void reconnectTimerTimeout ();

};

#endif
