#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QObject>

class QTimer;

/**
  * FIXME: the state is not good yet
 *   - it's too complex?
 *   - we would like to notify the user about connection failed/connection lost,
 *     with reconnect time ("reconnect in 4s") and the reason (click on label)
 * We should probably have
 *   - isOpen
 *   - connectionState (disconnected, connecting, connected, waitReconnect)
 *   - reconnectTime
 *   - error (noError, connectionFailed, connectionLost)
 *   - errorMessage
 *   - dataState (noData, good, timeout)
 *
 * FIXME: the implementations should not emit the signals themselves
 *
 * FIXME: for debugging, dump the state on change (e. g. in MainWindow)
 *
 * TODO:
 *   - it would be better to have the implementation just send data and split
 *     it into lines in this class
 */
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

				bool isOpen          () const { return open;            }
				DataStream::ConnectionState getConnectionState ()
				                        const { return connectionState; }
				bool getDataReceived () const { return dataReceived;    }
				bool getDataTimeout  () const { return dataTimeout;     }
				bool isDataValid     () const
				{
					if (connectionState!=connected) return false; // Connection not even opened
					return dataReceived && !dataTimeout;
				}

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
		void setOpen (bool o);

		State getState ();

	signals:
		void stateChanged (DataStream::State state);
		void lineReceived (const QString &line);

	protected:
		// Stream implementation
		virtual void openImplementation ()=0;
		virtual void closeImplementation ()=0;

		// Subclass interface
		virtual void dataReceived ();
		virtual void connectionOpening ();
		virtual void connectionEstablished ();
		virtual void connectionFailed ();
		virtual void connectionLost ();

	private:
	    QTimer *dataTimer;
	    QTimer *reconnectTimer;

	    State state;

	private slots:
		void dataTimerTimeout ();
		void reconnectTimerTimeout ();

};

#endif
