#include "src/io/dataStream/SerialDataStream.h"

#include <QDebug>
#include <QTimer>

// QtSerialPort
//#include <QtAddOnSerialPort/serialport.h>
//#include <QtAddOnSerialPort/serialportinfo.h>
//QT_USE_NAMESPACE_SERIALPORT

// QSerialDevice
#include "3rdparty/qserialdevice/src/qserialdevice/abstractserial.h"
#include "3rdparty/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator.h"

#include "src/i18n/notr.h"
#include "src/text.h"

// Improvements:
//   * When "our" port becomes available again, we should attempt to reconnect
//     immediately instead of waiting for the reconnect timer to expire.
//   * The time between plugging in a port and receiving the port changed signal
//     is relatively long.

SerialDataStream::SerialDataStream (QObject *parent): DataStream (parent),
	_baudRate (0)
{
	// Create the port and connect the required signals. _port will be deleted
	// automatically by its parent.
	// FIXME don't create in constructor - allow calling open() from a different
	// thread than the constructor
	// _port=new SerialPort (this); // QtSerialPort
	_port=new AbstractSerial (this); // QSerialDevice
	connect (_port, SIGNAL (readyRead         ()),
	         this , SLOT   (port_dataReceived ()));

	// QSerialDevice - not supported in QtSerialPort
	SerialDeviceEnumerator *serialDeviceEnumerator=SerialDeviceEnumerator::instance ();
	connect (serialDeviceEnumerator, SIGNAL (hasChanged            (const QStringList &)),
	         this                  , SLOT   (availablePortsChanged (const QStringList &)));
}

SerialDataStream::~SerialDataStream ()
{
}


// ****************
// ** Properties **
// ****************

/**
 * Sets the target port and baud rate.
 */
void SerialDataStream::setPort (const QString &portName, int baudRate)
{
	_baudRate=baudRate;
	_portName=portName;
}


// ************************
// ** DataStream methods **
// ************************

void SerialDataStream::openStream ()
{
	// Open the port
	//_port->clearError (); // QtSerialPort

	if (isBlank (_portName))
	{
		streamError (); // FIXME (tr ("No port specified"));
		return;
	}

	QStringList availableDevices=SerialDeviceEnumerator::instance ()->devicesAvailable ();
	if (!availableDevices.contains (_portName, Qt::CaseInsensitive))
	{
		streamError (); // FIXME (tr ("The port %1 does not exist").arg (_portName));
		return;
	}

	// QSerialDevice
	_port->setDeviceName (_portName);
	// QtSerialPort
	//_port->setPort (_portName);
	//if (_port->error ()!=SerialPort::NoError)
	//{
	//	connectionClosed (_port->errorString ());
	//	return;
	//}
	_port->open (QIODevice::ReadOnly);
	if (!_port->isOpen ())
	{
		streamError (); // FIXME (tr ("Connection did not open"));
		return;
	}

	// QtSerialPort
	//if (_port->error ()!=SerialPort::NoError)
	//{
	//	_port->close ();
	//	connectionClosed (_port->errorString ());
	//	return;
	//}

	// Configure the port. The port is currently configured as the last user of
	// the port left it.
	_port->setBaudRate    (_baudRate);
	_port->setDataBits    (AbstractSerial::DataBits8);
	_port->setParity      (AbstractSerial::ParityNone);
	_port->setStopBits    (AbstractSerial::StopBits1);
	_port->setFlowControl (AbstractSerial::FlowControlOff);
	_port->setDtr         (true);
	_port->setRts         (true);

	// QtSerialPort
//	if (_port->error ()!=SerialPort::NoError)
//	{
//		_port->close ();
//		connectionClosed (_port->errorString ());
//		return;
//	}
//	_port->clearError ();

	streamOpened ();
}

void SerialDataStream::closeStream ()
{
	_port->close ();
}


// *****************
// ** Port events **
// *****************

/**
 * Called when data is received from the port
 */
void SerialDataStream::port_dataReceived ()
{
	dataReceived (_port->readAll ());
}

void SerialDataStream::availablePortsChanged (const QStringList &ports)
{
	// QSerialDevice - not supported in QtSerialPort
	if (_port->isOpen ())
	{
		if (!ports.contains (_portName, Qt::CaseInsensitive))
		{
			// Oops - our port is not available any more
			_port->close ();
			streamError (); // FIXME (tr ("The port is no longer available"));
		}
	}
}
