#include "src/nmea/GpsTracker.h"

#include <QTimer>

#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GprmcSentence.h"

/**
 * Creates a GpsTracker instance
 *
 * @param parent the Qt parent object. This object will be deleted automatically
 *               when the parent is destroyed. Can be NULL.
 */
GpsTracker::GpsTracker (QObject *parent): QObject (parent),
	nmeaDecoder (NULL)
{
	timer=new QTimer (this); // Will be deleted by its parent (this)
	timer->setInterval (4000); // FIXME! configurable? At least symbolic constant.
	connect (timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
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

	timer->start ();

	bool positionHasChanged=(sentence.position!=this->position);

	this->gpsTime=sentence.timestamp;
	this->position=sentence.position;

	if (positionHasChanged)
		emit positionChanged (this->position);
}

void GpsTracker::timeout ()
{
	bool positionHasChanged=this->position.isValid ();

	this->position=GeoPosition ();

	if (positionHasChanged)
		emit positionChanged (this->position);
}
