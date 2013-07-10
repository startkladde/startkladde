#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QObject>

class QTimer;

// FIXME this (and implementations) must be thread safe
class DataStream: public QObject
{
	Q_OBJECT

	public:
		enum State { closedState, openingState, openState };

		DataStream (QObject *parent);
		virtual ~DataStream ();

		State getState ();
		static QString stateText (State state);

	public slots:
		virtual void open ();
		virtual void close ();
		virtual void setOpen (bool o);

	signals:
		void stateChanged ();
		void dataReceived (QByteArray data);

	protected:
		// Implementation interface
		virtual void openStream ()=0;
		virtual void closeStream ()=0;

		virtual void streamOpened ();
		virtual void streamError ();

		virtual void streamDataReceived (const QByteArray &data);

	private:
		void setState (State state);

		State _state;
		QString _buffer;
};

#endif
