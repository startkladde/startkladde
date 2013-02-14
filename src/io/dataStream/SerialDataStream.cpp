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

// Improvements:
//   * When "our" port becomes available again, we should attempt to reconnect
//     immediately instead of waiting for the reconnect timer to expire.
//   * The time between plugging in a port and receiving the port changed signal
//     is relatively long.

SerialDataStream::SerialDataStream ():
	_baudRate (0)
{
	// Note that we don't send a signal for the initial connection state. This
	// is the constructor, so no connection can have been made at this point.

	// Create the port and connect the required signals. _port will be deleted
	// automatically by its parent.
	// _port=new SerialPort (this); // QtSerialPort
	_port=new AbstractSerial (this); // QSerialDevice
	connect (_port, SIGNAL (readyRead ()), this, SLOT (portDataReceived ()));

	// QSerialDevice - not supported in QtSerialPort
	connect (SerialDeviceEnumerator::instance (), SIGNAL (hasChanged (const QStringList &)), this, SLOT (availablePortsChanged (const QStringList &)));
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
	//_port->clearError (); // QtSerialPort

	QStringList availableDevices=SerialDeviceEnumerator::instance ()->devicesAvailable ();
	if (!availableDevices.contains (_portName, Qt::CaseInsensitive))
	{
		qDebug () << "The port does not exist";
		connectionClosed (tr ("The port does not exist"));
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
	// QtSerialPort
	//if (_port->error ()!=SerialPort::NoError)
	//{
	//	_port->close ();
	//	connectionClosed (_port->errorString ());
	//	return;
	//}

	// Setup the port. The port is currently configured as the last user of the
	// port left it.
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
			connectionClosed (tr ("The port is no longer available"));
		}
	}
}
