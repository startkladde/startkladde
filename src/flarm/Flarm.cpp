#include "src/flarm/Flarm.h"

#include "src/config/Settings.h"
#include "src/flarm/FlarmList.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/io/dataStream/SerialDataStream.h"
#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GpsTracker.h"

Flarm::Flarm (QObject *parent, DbManager &dbManager): QObject (parent),
	_dbManager (dbManager),
	_dataStream (NULL), _open (false)
{
	// NMEA decoder
	// The data stream will be created and connected to the NMEA decoder later,
	// because the kind of stream may change.
	_nmeaDecoder=new NmeaDecoder ();

	// GPS tracker
	_gpsTracker=new GpsTracker (this);
	_gpsTracker->setNmeaDecoder (_nmeaDecoder);

	// Flarm list
	_flarmList=new FlarmList (this);
	_flarmList->setNmeaDecoder (_nmeaDecoder);
	_flarmList->setDatabase (&dbManager);

	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));
}

Flarm::~Flarm ()
{

}

// ****************
// ** Connection **
// ****************

void Flarm::open  () { setOpen (true ); }
void Flarm::close () { setOpen (false); }

void Flarm::setOpen (bool o)
{
	_open=o;
	updateOpen ();
}

template<class T> T *Flarm::ensureTypedDataStream ()
{
	T *typedDataStream=dynamic_cast<T *> (_dataStream);

	if (!typedDataStream)
	{
		// Either there is no data stream, or it has the wrong type. Delete the
		// old stream (if any), then create a new one with the correct type and
		// store it in the class property.
		delete _dataStream;
		typedDataStream=new T ();
		_dataStream=typedDataStream;
	}

	return typedDataStream;
}

void Flarm::updateOpen ()
{
	Settings &s=Settings::instance ();

	if (_open && s.flarmEnabled)
	{
		// Flarm stream
		switch (s.flarmConnectionType)
		{
			case Flarm::noConnection:
				// Delete the stream, if any. This will also disconnect its
				// signals.
				delete _dataStream;
				_dataStream=NULL;
				break;
			case Flarm::serialConnection:
				// FIXME enable
//				ensureTypedDataStream<SerialDataStream> ()->setPort (s.flarmSerialPort, s.flarmSerialBaudRate);
				delete _dataStream;
				_dataStream=NULL;
				break;
			case Flarm::tcpConnection:
				ensureTypedDataStream<TcpDataStream> ()->setTarget (s.flarmTcpHost, s.flarmTcpPort);
				break;
		}

		if (_dataStream)
		{
			connect (
				_dataStream , SIGNAL (lineReceived (const QString &)),
				_nmeaDecoder, SLOT   (lineReceived (const QString &)));

			connect (
				_dataStream, SIGNAL (stateChanged       (DataStream::State)),
				this       , SIGNAL (streamStateChanged (DataStream::State)));
			emit streamStateChanged (_dataStream->state ());

			_dataStream->open ();
		}
	}
	else
	{
		delete _dataStream;
		_dataStream=NULL;
	}

}


// **************
// ** Settings **
// **************

void Flarm::settingsChanged ()
{
	// The "Flarm enabled" setting may have changed, so we may have to open or
	// close the connection. Same for the connection type and parameters.
	updateOpen ();
}


// ************************
// ** Data stream facade **
// ************************

bool Flarm::isDataValid ()
{
	return _dataStream->state ().isDataOk ();
}


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
		// no default
	}

	return "unknown";
}

Flarm::ConnectionType Flarm::ConnectionType_fromString (const QString &string, ConnectionType defaultValue)
{
	if      (string=="none"  ) return noConnection;
	else if (string=="serial") return serialConnection;
	else if (string=="tcp"   ) return tcpConnection;
	else                       return defaultValue;
}

QString Flarm::ConnectionType_text (ConnectionType type)
{
	switch (type)
	{
		case noConnection     : return tr ("None");
		case serialConnection : return tr ("Serial/USB/Bluetooth");
		case tcpConnection    : return tr ("Network (TCP)");
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

	return list;
}

