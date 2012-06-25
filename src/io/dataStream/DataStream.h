#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QObject>

class DataStream: public QObject
{
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
};

#endif
