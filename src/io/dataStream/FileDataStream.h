#ifndef FILEDATASTREAM_H_
#define FILEDATASTREAM_H_

class QFile;
class QTimer;

#include "src/io/dataStream/DataStream.h"

/**
 * A DataStream implementation that reads data from a file, one line at a time,
 * with a constant, configurable delay between lines.
 *
 * The file name and delay are configured using the setFileName and setDelay
 * methods. After that, the stream can be opened.
 *
 * This DataStream implementation will block on opening until the file has been
 * opened, which should typically be instantaneous; however, if the file is on
 * a slow medium (such as a network path), this may take some time.
 */
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
