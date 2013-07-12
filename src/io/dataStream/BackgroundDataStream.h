#ifndef BACKGROUNDDATASTREAM_H_
#define BACKGROUNDDATASTREAM_H_

#include <QObject>

#include "src/io/dataStream/DataStream.h"

class BackgroundDataStream: public QObject
{
	Q_OBJECT

	public:
		// FIXME disallow copying
		BackgroundDataStream (QObject *parent, DataStream *stream, bool streamOwned);
		virtual ~BackgroundDataStream ();

		DataStream *getStream () const;

		DataStream::State getState () const;

	public slots:
		// Public interface
		virtual void open ();
		virtual void close ();
		virtual void setOpen (bool o);

	signals:
		// Public interface
		void stateChanged (DataStream::State state);
		void dataReceived (QByteArray data);

		// Signals to the underlying stream
		void open_stream ();
		void close_stream ();
		void setOpen_stream (bool o);

	private slots:
		// Signals from the underlying stream. Most of the signals are simply
		// re-emitted.
		void stream_stateChanged (DataStream::State state);


	private:
		QThread *_thread;

		DataStream *_stream;
		bool _streamOwned;

		DataStream::State _state;
};

#endif
