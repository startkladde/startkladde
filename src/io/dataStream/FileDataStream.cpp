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
	bool changed=false;
	if (fileName!=_fileName) changed=true;

	_fileName=fileName;

//	if (changed)
//		parametersChanged ();
}

void FileDataStream::setDelay (int milliseconds)
{
	bool changed=false;
	if (milliseconds!=_delayMs) changed=true;

	_delayMs=milliseconds;

//	if (changed)
//		parametersChanged ();
}


// ************************
// ** DataStream methods **
// ************************

void FileDataStream::openStream ()
{
	qDebug () << "FileDataStream: pre-open delay";
	DefaultQThread::sleep (2);

//	qDebug () << "FileDataStream: openStream";
	if (_file->isOpen ())
		_file->close ();

	_file->setFileName (_fileName);
	bool success=_file->open (QFile::ReadOnly | QFile::Text);

	if (success)
	{
		_timer->setInterval (_delayMs);
		_timer->start ();
		streamOpened ();
	}
	else
	{
		streamError ();
	}
}

void FileDataStream::closeStream ()
{
//	qDebug () << "FileDataStream: closeStream";
	_file->close ();
	_timer->stop ();
}


// ***********
// ** Timer **
// ***********

void FileDataStream::timerSlot ()
{
	if (_file->isOpen ())
	{
		QByteArray line=_file->readLine();
		if (line.isEmpty ())
		{
			qDebug () << "FileDataStream: EOF";
			// End of file reached
			_timer->stop ();
			_file->close ();
			streamError ();
		}
		else
		{
			qDebug () << "FileDataStream: data - " << line.trimmed ();
			emit dataReceived (line);
		}
	}
	else
	{
		streamError ();
	}
}
