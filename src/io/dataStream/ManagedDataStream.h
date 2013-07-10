#ifndef MANAGEDDATASTREAM_H_
#define MANAGEDDATASTREAM_H_

#include <QObject>

class QTimer;

class DataStream;

class ManagedDataStream: public QObject
{
	Q_OBJECT

	public:
		// Construction
		ManagedDataStream (QObject *parent);
		virtual ~ManagedDataStream ();

		void setDataStream (DataStream *stream, bool streamOwned);
		DataStream *getDataStream () const;

	public slots:
		// Opening/closing
		void open ();
		void close ();
		void setOpen (bool o);
		bool isOpen () const;

	signals:
		// Public interface
		void dataReceived (QString line);
		void lineReceived (QString line);

	private:
		DataStream *_stream;
		bool _streamOwned;

		bool _open;

	    QTimer *_dataTimer;
	    QTimer *_reconnectTimer;

	    QString buffer;

	private slots:
		void dataTimer_timeout ();
		void reconnectTimer_timeout ();

		void stream_stateChanged ();
		void stream_dataReceived (QByteArray data);
};

#endif
