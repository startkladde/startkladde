#ifndef MANAGEDDATASTREAM_H_
#define MANAGEDDATASTREAM_H_

#include <QObject>
#include <QTime>

#include "src/io/dataStream/DataStream.h" // For DataStream::State

class QTimer;

class BackgroundDataStream;

// FIXME documentation
class ManagedDataStream: public QObject
{
	Q_OBJECT

	public:
		struct State { enum Type { closed, opening, open, ok, timeout, error }; };

		// Construction
		ManagedDataStream (QObject *parent);
		virtual ~ManagedDataStream ();

		// State
		State::Type getState () const;
		static QString stateText (State::Type state);
		QString getErrorMessage () const;
		QTime getReconnectTime () const;

		// Underlying data stream
		void setDataStream (DataStream *stream, bool streamOwned);
		void clearDataStream ();
		DataStream *getDataStream () const;

	public slots:
		// Opening/closing
		void open ();
		void close ();
		void setOpen (bool o);
		bool isOpen () const;

	signals:
		// Public interface
		void stateChanged (ManagedDataStream::State::Type state);
		void dataReceived (QByteArray line);
		void connectionBecameAvailable ();

	protected:
		void goToState (State::Type state);
		void startReconnectTimer ();

	private:
		BackgroundDataStream *_backgroundStream;

		// Configuration
		bool _open;

		// State
		State::Type _state;

		// Internals
	    QTimer *_dataTimer;
	    QTimer *_reconnectTimer;
	    QTime _reconnectTime;

	private slots:
		void dataTimer_timeout ();
		void reconnectTimer_timeout ();

		void stream_stateChanged (DataStream::State state);
		void stream_dataReceived (QByteArray data);
		void stream_connectionBecameAvailable ();
};

#endif
