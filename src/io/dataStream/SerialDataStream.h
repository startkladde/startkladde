#ifndef SERIALDATASTREAM_H_
#define SERIALDATASTREAM_H_

#include <QStringList>

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
		// Stream implementation
		virtual void openConnection  ();
		virtual void closeConnection ();

	private:
		// QtSerialPort
		//QtAddOn::SerialPort::SerialPort *_port;
		// QSerialDevice
		AbstractSerial *_port;

		QString _portName;
		int _baudRate;

	private slots:
		void availablePortsChanged (const QStringList &ports);
    	void portDataReceived ();
};

#endif
