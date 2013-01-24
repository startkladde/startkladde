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

		void setPort (const QString &portName, int port);

	protected:
		void openImplementation ();
		void closeImplementation ();

	private:
		QtAddOn::SerialPort::SerialPort *port;

	private slots:
    	void portDataReceived ();
};

#endif
