#ifndef SERIALDATASTREAM_H_
#define SERIALDATASTREAM_H_

#include <QStringList>
#include <QDateTime>

#include "src/io/dataStream/DataStream.h"

// QtSerialPort
//namespace QtAddOn { namespace SerialPort { class SerialPort; } }
// QSerialDEvice
class AbstractSerial;

class SerialDataStream: public DataStream
{
	Q_OBJECT

	public:
		SerialDataStream (QObject *parent);
		virtual ~SerialDataStream ();

		void setPort (const QString &portName, int baudRate);

	protected:
		// DataStream methods
		virtual void openStream ();
		virtual void closeStream ();

	private:
		// QtSerialPort
		//QtAddOn::SerialPort::SerialPort *_port;
		// QSerialDevice
		AbstractSerial *_port;

		QString _portName;
		int _baudRate;

	private slots:
		void availablePortsChanged (const QStringList &ports);
    	void port_dataReceived ();
		void port_status (const QString &status, QDateTime dateTime);
};

#endif
