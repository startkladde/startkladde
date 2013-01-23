#include <QDebug>

#include "src/io/dataStream/SerialDataStream.h"

#include <QtAddOnSerialPort/serialport.h>
#include <QtAddOnSerialPort/serialportinfo.h>

QT_USE_NAMESPACE_SERIALPORT

//foreach (const SerialPortInfo &info, SerialPortInfo::availablePorts()) {
//    qDebug () << info.portName () << info.description () << info.manufacturer ();
//
//    // Example use SerialPort
//    SerialPort serial;
//    serial.setPort(info);
//    if (serial.open(QIODevice::ReadWrite))
//        serial.close();
//}
//

SerialDataStream::SerialDataStream ()
{
	// Note that we don't send a signal for the initial connection state. This
	// is the constructor, so no connection can have been made at this point.

	// Create the port and connect the required signals
	port=new SerialPort (this);
	connect (port, SIGNAL (readyRead ()), this, SLOT (portDataReceived ()));
	// FIXME! error handling
}

SerialDataStream::~SerialDataStream ()
{
}


// ****************
// ** Properties **
// ****************

void SerialDataStream::setPortName (const QString &portName)
{
	port->setPort (portName);
}

void SerialDataStream::setBaudRate (const int baudRate)
{
	port->setRate (baudRate);
}

void SerialDataStream::setPort (const QString &portName, int baudRate)
{
	port->setPort (portName);
	port->setRate (baudRate);
}


// ************************
// ** DataStream methods **
// ************************

void SerialDataStream::openImplementation ()
{
	// Nothing to do if the connection is already open or currently opening
	if (!port->isOpen ())
	{
		port->open (QIODevice::ReadOnly);
		connectionOpening ();
	}
}

void SerialDataStream::closeImplementation ()
{
	// We can do this even if the connection is not open (FIXME really?)
	port->close ();
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
	dataReceived ();

	// Read lines from the socket and emit them
	while (port->canReadLine ())
	{
		QString line = port->readLine ().trimmed ();
		emit lineReceived (line);
	}
}
