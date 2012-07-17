#include "GprmcSentence.h"

#include <QStringList>

#include "src/nmea/Nmea.h"

GprmcSentence::GprmcSentence (const QString &line): NmeaSentence (line, "GPRMC", 10),
	status (false)
{
	if (!isValid ()) return;
	QStringList parts = getParts ();

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
}

GprmcSentence::~GprmcSentence ()
{
}
