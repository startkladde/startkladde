#include "src/flarm/Flarm.h"

#include "src/config/Settings.h"
#include "src/flarm/FlarmList.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GpsTracker.h"

Flarm::Flarm (QObject *parent, DbManager &dbManager): QObject (parent),
	_dbManager (dbManager),
	_open (false)
{
	// Flarm stream
	_dataStream=new TcpDataStream ();
	_dataStream->setTarget ("localhost", 4711);

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
