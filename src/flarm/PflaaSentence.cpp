#include <src/flarm/PflaaSentence.h>

#include <QStringList>
#include <QDebug>

#include "src/flarm/Nmea.h"

PflaaSentence::PflaaSentence (const QString &sentence):
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

	// Make sure that it is actually a PFLAA sentence
	if (parts[0]!="$PFLAA") return;

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

	isValid=true;
}

PflaaSentence::~PflaaSentence ()
{
}
