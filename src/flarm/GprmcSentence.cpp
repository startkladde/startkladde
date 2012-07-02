#include <src/flarm/GprmcSentence.h>

#include <QStringList>
#include <QDebug>

#include "src/flarm/Nmea.h"

GprmcSentence::GprmcSentence (const QString &sentence):
	sentence (sentence), isValid (false)
{
	// Make sure that the checksum is valid
	if (!Nmea::verifyChecksum (sentence))
	{
		qDebug () << QString ("Checksum mismatch: "+sentence);
		return;
	}

	// Split the string
	// FIXME remove the checksum from the last part
	QStringList parts = sentence.split (',');

	// Make sure that it is not truncated
	if (parts.length () < 10) return;

	// Make sure that it is actually a GPRMC sentence
	if (parts[0]!="$GPRMC") return;

	timestamp = Nmea::parseDateTime (parts[9], parts[1]);
	status    = parts[2]=="A";
	Angle latitude  = Nmea::parseLatitude  (parts[3], parts[4]);
	Angle longitude = Nmea::parseLongitude (parts[5], parts[6]);
	position=GeoPosition (latitude, longitude);
	// parts[7]: velocity
	// parts[8]: true track in degrees
	// parts[9]: date (already used)
	// parts[10]: variation
	// parts[11]: variationSign
	// parts[12]: integrity

	// FIXME validity
	isValid=true;
}

GprmcSentence::~GprmcSentence ()
{
}
