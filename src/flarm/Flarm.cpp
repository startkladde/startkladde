#include "src/flarm/Flarm.h"

#include "src/config/Settings.h"
#include "src/flarm/FlarmList.h"
#include "src/io/dataStream/FileDataStream.h"
#include "src/io/dataStream/ManagedDataStream.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/io/dataStream/SerialDataStream.h"
#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GpsTracker.h"

// Data flow:
//   Data stream --> NMEA decoder --.--> GPS tracker
//                                  '--> Flarm list

Flarm::Flarm (QObject *parent, DbManager &dbManager): QObject (parent),
	_dbManager (dbManager)
{
	// Create the managed data stream (but do not create the underlying data
	// stream at this point). _managedDataStream will be deleted automatically
	// by Qt.
	_managedDataStream=new ManagedDataStream (this);

	// Create the NMEA decoder
	// The data stream will be created and connected to the NMEA decoder later,
	// because the stream may change at runtime, depending on the configuration.
	_nmeaDecoder=new NmeaDecoder (this);

	// Create the GPS tracker and connect it to the NMEA decoder
	_gpsTracker=new GpsTracker (this);
	_gpsTracker->setNmeaDecoder (_nmeaDecoder);

	// Create the Flarm list and connect it to the NMEA decoder
	_flarmList=new FlarmList (this);
	_flarmList->setNmeaDecoder (_nmeaDecoder);
	_flarmList->setDatabase (&dbManager);

	connect (_managedDataStream, SIGNAL (lineReceived (const QString &)),
	         _nmeaDecoder      , SLOT   (processLine  (const QString &)));

	// When the settings change, we may have to react
	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));

	updateDataStream ();
}

Flarm::~Flarm ()
{
	// _dataStream, _nmeaDecoder, _gpsTracker and _flarmList will be deleted
	// automatically by Qt.
}

// ****************
// ** Connection **
// ****************

// Forward open/close methods to the managed data stream.
void Flarm::open    ()       { _managedDataStream->open    ();  }
void Flarm::close   ()       { _managedDataStream->close   ();  }
void Flarm::setOpen (bool o) { _managedDataStream->setOpen (o); }
bool Flarm::isOpen  () const { return _managedDataStream->isOpen (); }

template<class T> T *Flarm::ensureTypedDataStream ()
{
	// Try to cast the existing data stream (if any) to the requested type.
	T *typedDataStream=dynamic_cast<T *> (_managedDataStream->getDataStream ());

	// If the cast returned NULL, there is either no data stream, or it has the
	// wrong type. In this case, create
	if (!typedDataStream)
	{
		// Either there is no data stream, or it has the wrong type. Create a
		// new one data stream with the correct type and store it.
		typedDataStream=new T (this);
		_managedDataStream->setDataStream (typedDataStream, true);
	}

	// Return the pre-existing or newly created data stream
	return typedDataStream;
}

void Flarm::updateDataStream ()
{
	// Make sure that the data stream has the correct type or is NULL, depending
	// on the configuration.
	Settings &s=Settings::instance ();
	if (s.flarmEnabled)
	{
		// Flarm stream
		switch (s.flarmConnectionType)
		{
			case Flarm::noConnection:
			{
				_managedDataStream->clearDataStream ();
			} break;
			case Flarm::serialConnection:
			{
//				SerialDataStream *stream=ensureTypedDataStream<SerialDataStream> ();
//				stream->setPort (s.flarmSerialPort, s.flarmSerialBaudRate);
				_managedDataStream->clearDataStream ();
			} break;
			case Flarm::tcpConnection:
			{
//				TcpDataStream *stream=ensureTypedDataStream<TcpDataStream> ();
//				stream->setTarget (s.flarmTcpHost, s.flarmTcpPort);
				_managedDataStream->clearDataStream ();
			} break;
			case Flarm::fileConnection:
			{
				FileDataStream *stream=ensureTypedDataStream<FileDataStream> ();
				stream->setFileName (s.flarmFileName);
				stream->setDelay (s.flarmFileDelayMs);
			} break;
			// no default
		}
	}
	else
	{
		_managedDataStream->clearDataStream ();
	}
}


// **************
// ** Settings **
// **************

void Flarm::settingsChanged ()
{
	// The Flarm enabled, connection type or connection parameters may have
	// changed. We may have to change the connection type.
	updateDataStream ();
}


// *****************
// ** Data stream **
// *****************

bool Flarm::isDataValid ()
{
	// FIXME implement
	return false;
//	if (dataStream ())
//		return dataStream ()->state ().isDataOk ();
//	else
//		return false;
}

//void Flarm::dataStream_stateChanged (DataStream::State state)
//{
//	// FIXME implement
//	(void)state;
////	emit connectionStateChanged (connectionState ());
//}


// ************************
// ** GPS tracker facade **
// ************************

GeoPosition Flarm::getPosition () const { return _gpsTracker->getPosition (); }
QDateTime   Flarm::getGpsTime  () const { return _gpsTracker->getGpsTime  (); }


// *********************
// ** Connection type **
// *********************

QString Flarm::ConnectionType_toString (ConnectionType type)
{
	switch (type)
	{
		case noConnection     : return "none";
		case serialConnection : return "serial";
		case tcpConnection    : return "tcp";
		case fileConnection   : return "file";
		// no default
	}

	return "unknown";
}

Flarm::ConnectionType Flarm::ConnectionType_fromString (const QString &string, ConnectionType defaultValue)
{
	if      (string=="none"  ) return noConnection;
	else if (string=="serial") return serialConnection;
	else if (string=="tcp"   ) return tcpConnection;
	else if (string=="file"  ) return fileConnection;
	else                       return defaultValue;
}

QString Flarm::ConnectionType_text (ConnectionType type)
{
	switch (type)
	{
		case noConnection     : return tr ("None");
		case serialConnection : return tr ("Serial/USB/Bluetooth");
		case tcpConnection    : return tr ("Network (TCP)");
		case fileConnection   : return tr ("File");
		// no default
	}

	return "unknown";
}

QList<Flarm::ConnectionType> Flarm::ConnectionType_list ()
{
	QList<ConnectionType> list;

	list << noConnection;
	list << serialConnection;
	list << tcpConnection;
	list << fileConnection;

	return list;
}

