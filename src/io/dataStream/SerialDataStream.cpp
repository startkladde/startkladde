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

//	QTimer *timer=new QTimer (this);
//	connect (timer, SIGNAL (timeout ()), this, SLOT (checkPort ()));
//	timer->setSingleShot (false);
//	timer->setInterval (1000);
//	timer->start ();
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

void SerialDataStream::openConnection ()
{
	// Open the port
	_port->clearError ();

	_port->setPort (_portName);
	if (_port->error ()!=SerialPort::NoError)
	{
		connectionClosed (_port->errorString ());
		return;
	}

	_port->open (QIODevice::ReadOnly);
	if (_port->error ()!=SerialPort::NoError)
	{
		_port->close ();
		connectionClosed (_port->errorString ());
		return;
	}

	// Setup the port. The port is currently configured as the last user of the
	// port left it.
	_port->setRate        (_baudRate);
	_port->setDataBits    (SerialPort::Data8);
	_port->setParity      (SerialPort::NoParity);
	_port->setStopBits    (SerialPort::OneStop);
	_port->setFlowControl (SerialPort::NoFlowControl);
	_port->setDtr         (true);
	_port->setRts         (true);
	if (_port->error ()!=SerialPort::NoError)
	{
		_port->close ();
		connectionClosed (_port->errorString ());
		return;
	}

	_port->clearError ();
	connectionOpened ();
}

void SerialDataStream::closeConnection ()
{
	_port->close ();
}


// *****************
// ** Port events **
// *****************

/**
 * Called when data is received from the port
 */
void SerialDataStream::portDataReceived ()
{
	// FIXME error handling - also, we keep getting serial error 10 (unknown
	// error although the data is received correctly).
//	if (_port->error ()!=SerialPort::NoError)
//		qDebug () << "Serial error:" << _port->error ();

	dataReceived (_port->readAll ());
}
