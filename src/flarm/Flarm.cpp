#include "src/flarm/Flarm.h"

#include "src/config/Settings.h"
#include "src/flarm/FlarmList.h"
#include "src/io/dataStream/TcpDataStream.h"
//#include "src/io/dataStream/SerialDataStream.h"
#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GpsTracker.h"

Flarm::Flarm (QObject *parent, DbManager &dbManager): QObject (parent),
	_dbManager (dbManager),
	_open (false)
{
	// Flarm stream
	TcpDataStream *dataStream=new TcpDataStream ();
	dataStream=new TcpDataStream ();
	dataStream->setTarget ("localhost", 4711);
	_dataStream=dataStream;

//	SerialDataStream *dataStream=new SerialDataStream ();
//	dataStream->setPort ("COM9", 19200);
//	_dataStream=dataStream;

	// NMEA decoder
	_nmeaDecoder=new NmeaDecoder ();
	connect (
		_dataStream , SIGNAL (lineReceived (const QString &)),
		_nmeaDecoder, SLOT   (lineReceived (const QString &)));

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

void Flarm::updateOpen ()
{
	Settings &s=Settings::instance ();
	_dataStream->setOpen (_open && s.flarmEnabled);
}


// **************
// ** Settings **
// **************

void Flarm::settingsChanged ()
{
	// The "Flarm enabled" setting may have changed, so we may have to open or
	// close the connection
	updateOpen ();
}


// ************************
// ** Data stream facade **
// ************************

bool Flarm::isDataValid ()
{
	return _dataStream->getState ().isDataValid ();
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

