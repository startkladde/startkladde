/*
 * Longitude.cpp
 *
 *  Created on: 13.07.2010
 *      Author: Martin Herrmann
 */

#include "Longitude.h"

#include <cstdlib>
#include <cassert>

#include <QRegExp>
#include <QDebug>

#include "src/util/qString.h"
#include "src/text.h"

Longitude::Longitude ():
	degrees (0), minutes (0), seconds (0), positive (true)
{
}

Longitude::Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive):
	degrees (degrees), minutes (minutes), seconds (seconds), positive (positive)
{
}

Longitude::~Longitude ()
{
}

QString Longitude::format (const QString &positiveString, const QString &negativeString) const
{
	return QString (utf8 ("%1° %2' %3\" %4"))
		.arg (abs (degrees))
		.arg (minutes)
		.arg (seconds)
		.arg (positive?positiveString:negativeString);
}

QString Longitude::toString () const
{
	return QString ("%1%2 %3 %4")
		.arg (positive?"":"-")
		.arg (degrees)
		.arg (minutes)
		.arg (seconds);
}

Longitude Longitude::fromString (const QString &string, bool *ok)
{
#define STOP do { if (ok) *ok=false; return Longitude (); } while (0)
	if (blank (string)) STOP;

	QRegExp re ("^([+-]?)\\s*(\\d*)\\s+(\\d*)\\s+(\\d*)$");
	bool numOk=true;

	if (!string.trimmed ().contains (re)) STOP;

	bool positive=!(re.cap (1)=="-");

	int degrees=re.cap (2).toInt (&numOk);
	if (!numOk) STOP;
	int minutes=re.cap (3).toUInt (&numOk);
	if (!numOk) STOP;
	int seconds=re.cap (4).toUInt (&numOk);
	if (!numOk) STOP;

	if (ok) *ok=true;
	return Longitude (degrees, minutes, seconds, positive);
}

void complementAngle (unsigned int &deg, unsigned int &min, unsigned int &sec)
{
	// deg in 180..359
	// min in 0..59
	// sec in 0..59
	// angle = deg° min' sec", not 180° 0' 0"

	//   360° - angle
	// = 360° - (deg° min' sec")
	// = (360° 00' 00") - (deg° min' sec")
	// = (359° 60' 00") - (deg° min' sec")
	// = (359° 59' 60") - (deg° min' sec")
	// = (359°-deg° 59'-min' 60"-sec")
	// = (359-deg)° (59-min)' (60-sec)"
	// where (60-sec) is in 1..60
	//       (59-min) is in 0..59
	//       (359-deg) is in 0..179
	//       angle is not 179° 59' 60"

	sec=60-sec;
	min=59-min;
	deg=359-deg;

	assert (sec>=1 && sec<=60);
	assert (min<=59);
	assert (deg<=179);
	assert (!(deg==179 && min==59 && sec==60));

	// We may have to renormalize
	if (sec==60) { sec=0; ++min; } // sec in 0..59, min in 0..60
	if (min==60) { min=0; ++deg; } // min in 0..59, deg in 0..179
	// deg cannot be 180 because that would have required 179° 59' 60"
}


// TODO this needs a unit test
Longitude Longitude::normalized () const
{
	unsigned int deg=degrees;
	unsigned int min=minutes;
	unsigned int sec=seconds;

	// Carry excess seconds to minutes (sec in 0..59)
	min+=sec/60;
	sec=sec%60;

	// Carry excess minutes to degrees (min in 0..59)
	deg+=min/60;
	min=min%60;

	// Normalize degrees (deg in 0..359)
	deg=deg%360;

	if (deg<180)
	{
		// Angles less than to 180° are returned with the same sign
		return Longitude (deg, min, sec, positive);
	}
	else if (deg==180 && min==0 && sec==0)
	{
		// Angles of exactly 180° are returned with a positive sign
		return Longitude (180, 0, 0, true);
	}
	else
	{
		// Other angles have to be mirrored (complement and invert sign)
		complementAngle (deg, min, sec);
		return Longitude (deg, min, sec, !positive);
	}
}

double Longitude::minusDegrees (const Longitude &other) const
{
	// Note the conversion from uint to int
	return
		((int)degrees-(int)other.degrees)
		+((int)minutes-(int)other.minutes)/60.0
		+((int)seconds-(int)other.seconds)/3600.0;
}
