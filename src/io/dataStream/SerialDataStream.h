#ifndef SERIALDATASTREAM_H_
#define SERIALDATASTREAM_H_

#include "src/io/dataStream/DataStream.h"

namespace QtAddOn { namespace SerialPort { class SerialPort; } }

class SerialDataStream: public DataStream
{
		Q_OBJECT

	public:
		SerialDataStream ();
		virtual ~SerialDataStream ();

		void setPort (const QString &portName, int baudRate);

	protected:
		// Stream implementation
		virtual void openConnection  ();
		virtual void closeConnection ();

	private:
		QtAddOn::SerialPort::SerialPort *_port;

		QString _portName;
		int _baudRate;

	private slots:
    	void portDataReceived ();
    	//void checkPort ();
};

#endif
