#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QBuffer>
#include <QObject>

class QTimer;

/**
 *
 */
class DataStream: public QObject
{
		Q_OBJECT

	public:
		enum StreamState
		{
			streamClosed,
			streamConnecting,
			streamNoData,
			streamDataOk,
			streamDataTimeout,
			streamConnectionError
		};

		enum ErrorType
		{
			noError,
			connectionFailedError,
			connectionLostError
		};

		struct State
		{
			// State
			StreamState streamState;

			// Auxiliary data, state dependent
			int timeRemaining;
			ErrorType errorType;
			QString errorMessage;

			// State properties
			bool isOpen      () { return streamState != streamClosed;      }
			bool isConnected () { return streamState == streamNoData ||
			                             streamState == streamDataOk ||
			                             streamState == streamDataTimeout; }
			bool isDataOk    () { return streamState == streamDataOk;      }

			bool operator== (const State &other) const;
			bool operator!= (const State &other) const;
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
		void stateChanged (DataStream::State state);
		void lineReceived (QString line);

	protected:
		// Stream implementation
		virtual void openConnection  ()=0;
		virtual void closeConnection ()=0;

		// Implementation interface
		void connectionOpened ();
		void connectionClosed (const QString &message);
		void dataReceived (const QByteArray &data);
		void parametersChanged ();

	private:
		State _state;

	    QTimer *dataTimer;
	    QTimer *reconnectTimer;

	    QString buffer;

	    void setState (StreamState streamState, int timeRemaining=0, ErrorType errorType=noError, const QString &errorMessage=QString ());

	private slots:
		void dataTimerTimeout ();
		void reconnectTimerTimeout ();

};

#endif
