#ifndef FILEDATASTREAM_H_
#define FILEDATASTREAM_H_

class QFile;
class QTimer;

#include "src/io/dataStream/DataStream.h"

class FileDataStream: public DataStream
{
	Q_OBJECT

	public:
		FileDataStream (QObject *parent);
		virtual ~FileDataStream ();

		void setFileName (const QString &fileName);
		void setDelay (int milliseconds);

	protected:
		// DataStream methods
		virtual void openStream ();
		virtual void closeStream ();

	private:
	    QFile *_file;
	    QTimer *_timer;

		QString _fileName;
		uint16_t _delayMs;

	private slots:
		void timerSlot ();
};

#endif
