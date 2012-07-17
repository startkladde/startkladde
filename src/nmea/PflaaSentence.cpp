#include "PflaaSentence.h"

#include <QStringList>

#include "src/nmea/Nmea.h"

PflaaSentence::PflaaSentence (const QString &line): NmeaSentence (line, "PFLAA", 10),
	relativeNorth (0), relativeEast (0), relativeVertical (0),
	groundSpeed (0), climbRate (0)
{
	if (!isValid ()) return;
	QStringList parts = getParts ();

	// FIXME ok flag

	// parts[1]: alarmLevel
	relativeNorth    = parts[ 2].toInt ();
	relativeEast     = parts[ 3].toInt ();
	relativeVertical = parts[ 4].toInt ();
	// parts[5]: idType
	flarmId          = parts[ 6];
	// parts[7]: true track in degrees
	// parts[8]: right turn rate in deg/s (currently omitted)
	groundSpeed      = parts[ 9].toInt ();
	climbRate        = parts[10].toDouble ();
	// parts[11]: aircraftType
}

PflaaSentence::~PflaaSentence ()
{
}
