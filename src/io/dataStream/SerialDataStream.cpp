#include "src/io/dataStream/SerialDataStream.h"

#include <QDebug>
#include <QTimer>

#include <QtAddOnSerialPort/serialport.h>
#include <QtAddOnSerialPort/serialportinfo.h>

QT_USE_NAMESPACE_SERIALPORT

SerialDataStream::SerialDataStream ():
	_baudRate (0)
{
	// Note that we don't send a signal for the initial connection state. This
	// is the constructor, so no connection can have been made at this point.

	// Create the port and connect the required signals. _port will be deleted
	// automatically by its parent.
	_port=new SerialPort (this);
	connect (_port, SIGNAL (readyRead ()), this, SLOT (portDataReceived ()));

	QTimer *timer=new QTimer (this);
	connect (timer, SIGNAL (timeout ()), this, SLOT (checkPort ()));
	timer->setSingleShot (false);
	timer->setInterval (1000);
	timer->start ();

}

SerialDataStream::~SerialDataStream ()
{
}


// ****************
// ** Properties **
// ****************

void SerialDataStream::setPort (const QString &portName, int baudRate)
{
	bool changed=false;
	if (baudRate!=_baudRate) changed=true;
	if (portName!=_portName) changed=true;

	_baudRate=baudRate;
	_portName=portName;

	if (changed)
		parametersChanged ();
}


// ************************
// ** DataStream methods **
// ************************

void SerialDataStream::openImplementation ()
{
	// Whatever, just make sure that it's open with the correct parameters
	// afterwards. We could avoid reopening it by checking whether the correct
	// port is currently open before.

	if (_port->isOpen ())
		_port->close ();

	// Open the port
	// FIXME! error handling, port->error () (must probably reset before)
	_port->clearError ();
	_port->setPort (_portName);
	_port->open (QIODevice::ReadOnly);
	if (_port->error ()!=SerialPort::NoError)
		qDebug () << "Serial open error:" << _port->error ();

	// Setup the port. The port is currently configured as the last user of the
	// port left it.
	_port->setRate        (_baudRate);
	_port->setDataBits    (SerialPort::Data8);
	_port->setParity      (SerialPort::NoParity);
	_port->setStopBits    (SerialPort::OneStop);
	_port->setFlowControl (SerialPort::NoFlowControl);
	_port->setDtr         (true);
	_port->setRts         (true);

	// FIXME this stinks
	connectionOpening ();
}

void SerialDataStream::closeImplementation ()
{
	// We can do this even if the connection is not open (FIXME really?)
	_port->close ();
}


// **********
// ** Data **
// **********

/**
 * Called when data is received from the port
 *
 * Updates the connection state, resets the data timer, and emits lines.
 */
void SerialDataStream::portDataReceived ()
{
	if (_port->error ()!=SerialPort::NoError)
		qDebug () << "Serial error:" << _port->error ();

	dataReceived ();

	// Read lines from the socket and emit them
	while (_port->canReadLine ())
	{
		QString line = _port->readLine ().trimmed ();
		emit lineReceived (line);
	}
}

void SerialDataStream::checkPort ()
{
	char data[1024];

	SerialPort::PortError errorBefore=_port->error ();
	int readResult=_port->read (data, 1024);
	SerialPort::PortError errorAfter=_port->error ();

	qDebug () << errorBefore << errorAfter << readResult;
}
