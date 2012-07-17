#include "src/nmea/GpsTracker.h"

#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GprmcSentence.h"

/**
 * Creates a GpsTracker instance
 *
 * @param parent the Qt parent object. This object will be deleted automatically
 *               when the parent is destroyed. Can be NULL.
 */
GpsTracker::GpsTracker (QObject *parent): QObject (parent),
	nmeaDecoder (NULL), altitude (0)
{
}

GpsTracker::~GpsTracker ()
{
	setNmeaDecoder (NULL);
}

/**
 * Sets the NMEA decoder to receive signals from
 */
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

/**
 * Get the most recent position received
 */
GeoPosition GpsTracker::getPosition () const
{
	return position;
}

/**
 * Gets the most recent GPS time received
 * @return
 */
QDateTime GpsTracker::getGpsTime () const
{
	return gpsTime;
}

/**
 * Updates the state based on a GPRMC sentence
 */
void GpsTracker::gprmcSentence (const GprmcSentence &sentence)
{
	if (!sentence.isValid ()) return;

	GeoPosition oldPosition=this->position;

	this->gpsTime=sentence.timestamp;
	this->position=sentence.position;
	// FIXME doesn't GPRMC have altitude? Get it from GPGGA.

	if (this->position != oldPosition)
		emit positionChanged (this->position);
}
