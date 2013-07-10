#include "ManagedDataStream.h"

#include <QDebug>
#include <QTimer>

#include "src/io/dataStream/DataStream.h"

// Implementation notes:
//   * We want to be able to assign a new DataStream instance to this class.
//     Therefore, we have to store the open/closed state (as requested by the
//     user) here rather than relying on the state of DataStream.



// ******************
// ** Construction **
// ******************

ManagedDataStream::ManagedDataStream (QObject *parent): QObject (parent),
	_stream (NULL), _streamOwned (false), _open (false)
{
	// Create the timers. The timers will be deleted automatically by Qt.
	_dataTimer      = new QTimer (this);
	_reconnectTimer = new QTimer (this);

    // Setup the timers
    _dataTimer     ->setInterval (2000); _dataTimer     ->setSingleShot (true);
    _reconnectTimer->setInterval (2000); _reconnectTimer->setSingleShot (true);

    // Connect the timers' signals
    connect (_dataTimer     , SIGNAL (timeout ()), this, SLOT (dataTimer_timeout      ()));
    connect (_reconnectTimer, SIGNAL (timeout ()), this, SLOT (reconnectTimer_timeout ()));

}

ManagedDataStream::~ManagedDataStream ()
{
	// FIXME can this interfere with the stream being deleted by its parent?
	if (_streamOwned)
		delete _stream;
}


// ****************************
// ** Underlying data stream **
// ****************************

void ManagedDataStream::setDataStream (DataStream *stream, bool streamOwned)
{
	qDebug () << "ManagedDataStream: set data stream";

	// If the stream is already current, stop
	if (_stream==stream)
		return;

	// Disconnect the old stream's signals
	if (_stream)
		_stream->disconnect (this);

	// Delete the old stream if we owned it
	if (_streamOwned)
		delete _stream;

	// Store the new stream
	_stream=stream;
	_streamOwned=streamOwned;

	if (_stream)
	{
		// Connect the new stream's signals
		connect (_stream, SIGNAL (stateChanged ()), this, SLOT (stream_stateChanged ()));
		connect (_stream, SIGNAL (dataReceived (QByteArray)), this, SLOT (stream_dataReceived (QByteArray)));

		// Update the new stream's state
		if (_open)
			_stream->open ();
		else
			_stream->close ();
	}
}

void ManagedDataStream::clearDataStream ()
{
	setDataStream (NULL, false);
}

DataStream *ManagedDataStream::getDataStream () const
{
	return _stream;
}


// *********************
// ** Opening/closing **
// *********************

void ManagedDataStream::open ()
{
	qDebug () << "ManagedDataStream: open";
	_open=true;

	if (_stream)
	{
		// Open the stream. When the connection succeeds or fails (now or later),
		// we will receive a signal. This may cause timer to be started.
		_stream->open ();
	}
}

void ManagedDataStream::close ()
{
	_open=false;

	// Stop the timers. Note that a timer event may still be in the event queue,
	// so timer slots may be invoked even when the connection is closed.
	_dataTimer     ->stop ();
	_reconnectTimer->stop ();

	if (_stream)
	{
		// Close the stream
		_stream->close ();
	}
}

void ManagedDataStream::setOpen (bool o)
{
	if (o)
		open ();
	else
		close ();
}

bool ManagedDataStream::isOpen () const
{
	return _open;
}


// ************
// ** Timers **
// ************

// When closing the stream, the timers are stopped. Still, it is possible to
// receive a timer event while the stream is closed, in the following case:
//   * timer expires, timer event is enqueued
//   * stream is closed
//   * timer event is received
// Therefore, it is necessary to check if the state is open in the timer slots.

void ManagedDataStream::reconnectTimer_timeout ()
{
	qDebug () << "ManagedDataStream: reconnect timeout";

	// Ignore if the stream is not open (see above)
	if (!_open)
		return;

	if (_stream)
	{
		_stream->open ();
	}
}

/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void ManagedDataStream::dataTimer_timeout ()
{
	qDebug () << "ManagedDataStream: data timeout";

	// Ignore if the stream is not open (see above)
	if (!_open)
		return;

	if (_stream)
	{

	}
}


// ************
// ** Stream **
// ************

void ManagedDataStream::stream_stateChanged ()
{
	DataStream::State state=_stream->getState ();

	qDebug () << "ManagedDataStream: stream state changed to" <<
		DataStream::stateText (state);

	switch (state)
	{
		case DataStream::closedState:
			qDebug () << "Yes, it's closed";
			if (_open)
			{
				qDebug () << "Start reconnect timer";
				_reconnectTimer->start ();
			}
			break;
		case DataStream::openingState:
			break;
		case DataStream::openState:
			break;
		// no default
	}
}

void ManagedDataStream::stream_dataReceived (QByteArray data)
{
	emit dataReceived (data);

	// Start or restart the data timer
	_dataTimer->start ();

	// FIXME this should be done either in NmeaDecoder (using a separate class)
	// or between the stream and the NmeaDecoder.
	// Process the data
	// Example:
	//   f o o \n b a r \n b
	//   0 1 2 3  4 5 6 7  8
	buffer.append (QString::fromUtf8 (data));
	int pos=0;
	while ((pos=buffer.indexOf ("\n"))>=0)
	{
		QString line=buffer.left (pos).trimmed ();
		buffer.remove (0, pos+1);
		emit lineReceived (line);
	}
}

