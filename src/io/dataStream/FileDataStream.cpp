#include "src/io/dataStream/FileDataStream.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "src/concurrent/DefaultQThread.h"

// ******************
// ** Construction **
// ******************

FileDataStream::FileDataStream (QObject *parent): DataStream (parent),
	_delayMs (0)
{
	// Create the file and the timer. _file and _timer will be deleted
	// automatically by its parent (this).
	_file=new QFile (this);
	_timer=new QTimer (this);

	connect (_timer, SIGNAL (timeout ()), this, SLOT (timerSlot ()));
}

FileDataStream::~FileDataStream ()
{
}


// ****************
// ** Properties **
// ****************

void FileDataStream::setFileName (const QString &fileName)
{
	_fileName=fileName;
}

void FileDataStream::setDelay (int milliseconds)
{
	_delayMs=milliseconds;
}


// ************************
// ** DataStream methods **
// ************************

void FileDataStream::openStream ()
{
	// A delay to simulate a blocking open method. This can be used to test a
	// background implementation.
//	qDebug () << "FileDataStream: pre-open delay";
//	DefaultQThread::sleep (2);

	// If the file is currently open, close it.
	if (_file->isOpen ())
		_file->close ();

	// Open the file for reading in text mode
	_file->setFileName (_fileName);
	bool success=_file->open (QFile::ReadOnly | QFile::Text);

	if (success)
	{
		// The file was successfully opened. Start the read timer with the
		// specified delay.
		_timer->setInterval (_delayMs);
		_timer->start ();
		streamOpened ();
	}
	else
	{
		// The file could not be opened.
		streamError (_file->errorString ());
	}
}

void FileDataStream::closeStream ()
{
	// Close the file and stop the timer. Note that there may still be a timer
	// event in the queue.
	_timer->stop ();
	_file->close ();
}


// ***********
// ** Timer **
// ***********

void FileDataStream::timerSlot ()
{
	// The file may have been closed after a timer event was queued. We
	// therefore have to verify that the file is still open.
	if (_file->isOpen ())
	{
		// Read a single line from the file. On EOF, this will return an empty
		// string.
		QByteArray line=_file->readLine ();

		if (line.isEmpty ())
		{
			// We reached the end of the file. Stop the timer and close the
			// file. We currently report this as an error; we could as well
			// silently rewind the file.
			//qDebug () << "FileDataStream: EOF";
			_timer->stop ();
			_file->close ();
			streamError (tr ("End of file reached"));
		}
		else
		{
			// Emit a signal with the received data.
			emit dataReceived (line);
		}
	}
}
