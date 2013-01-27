#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QBuffer>
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
 * FIXME: for debugging, dump the state on change (e. g. in MainWindow)
 */
class DataStream: public QObject
{
		Q_OBJECT

	public:
		enum StreamState
		{
			streamConnecting,
			streamConnected,
			streamConnectionFailed,
			streamConnectionLost
		};

		enum DataState
		{
			dataNone,
			dataOk,
			dataTimeout,
		};

		struct State
		{
			bool open;
			StreamState streamState;
			DataState dataState;

			bool isDataOk () { return open && streamState==streamConnected && dataState==dataOk; }
		};

		// Construction
		DataStream ();
		virtual ~DataStream ();

	public:
		// Public interface
		State state () const;

	public slots:
		// Public interface
		void open ();
		void close ();
		void setOpen (bool o);

	signals:
		// Public interface
		void stateChanged (State state);
		void lineReceived (QString line);

	protected:
		// Stream implementation
		virtual void openConnection  ()=0;
		virtual void closeConnection ()=0;

		// Implementation interface
		void connectionOpened ();
		void connectionClosed ();
		void dataReceived (const QByteArray &data);
		void parametersChanged ();

	private:
		State _state;

	    QTimer *dataTimer;
	    QTimer *reconnectTimer;

	    QString buffer;

	private slots:
		void dataTimerTimeout ();
		void reconnectTimerTimeout ();

};

#endif
