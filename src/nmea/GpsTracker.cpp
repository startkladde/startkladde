#include "src/nmea/GpsTracker.h"

#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GprmcSentence.h"

// FIXME add parent parameter
GpsTracker::GpsTracker (QObject *parent): QObject (parent),
	nmeaDecoder (NULL), altitude (0)
{
}

GpsTracker::~GpsTracker ()
{
	setNmeaDecoder (NULL);
}

void GpsTracker::setNmeaDecoder (NmeaDecoder *nmeaDecoder)
{
	if (nmeaDecoder==this->nmeaDecoder)
		return;

	if (this->nmeaDecoder)
	{
		disconnect (this->nmeaDecoder, SIGNAL (gprmcSentence (const GprmcSentence &)), this, SLOT (gprmcSentence (const GprmcSentence &)));
	}

	this->nmeaDecoder=nmeaDecoder;

	if (this->nmeaDecoder)
	{
		connect (this->nmeaDecoder, SIGNAL (gprmcSentence (const GprmcSentence &)), this, SLOT (gprmcSentence (const GprmcSentence &)));
	}
}

GeoPosition GpsTracker::getPosition ()
{
	return position;
}

QDateTime GpsTracker::getGpsTime ()
{
	return gpsTime;
}

// FIXME make sure only valid sentences are emitted
void GpsTracker::gprmcSentence (const GprmcSentence &sentence)
{
	this->gpsTime=sentence.timestamp;
	this->position=sentence.position;
	// FIXME doesn't GPRMC have altitude?

	emit positionChanged (this->position);
}
