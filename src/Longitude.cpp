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

/**
 * Creates a longitude representing
 *
 * @return
 */
Longitude::Longitude ():
	degrees (0), minutes (0), seconds (0), positive (true)
{
}

/**
 * Creates a longitude with given values
 */
Longitude::Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive):
	degrees (degrees), minutes (minutes), seconds (seconds), positive (positive)
{
}

Longitude::~Longitude ()
{
}

/**
 * Creates a string suitable for display to the user
 *
 * @param positiveString the designator to add for eastern longitudes
 * @param negativeString the designator to add for western longitudes
 * @return a QString containing a representation of this longitude
 */
QString Longitude::format (const QString &positiveString, const QString &negativeString) const
{
	return QString (utf8 ("%1° %2' %3\" %4"))
		.arg (abs (degrees))
		.arg (minutes)
		.arg (seconds)
		.arg (positive?positiveString:negativeString);
}

/**
 * Creates a string suitable for storage
 *
 * Consider using toDegrees instead
 *
 * @return a string suitable for storage
 * @see fromString
 * @see toDegrees
 */
QString Longitude::toString () const
{
	return QString ("%1%2 %3 %4")
		.arg (positive?"":"-")
		.arg (degrees)
		.arg (minutes)
		.arg (seconds);
}

/**
 * Creates a longitude by parsing a string created by toString
 *
 * @param string the string to parse. Leading and trailing whitespace is
 *        ignored.
 * @param ok if not NULL, set to true if the operation succeds and false else
 * @return the parse longitude, or a null longitude if the parsing failed
 * @see toString
 * @see fromDegrees
 */
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


/**
 * Determines the complement of the given angle, that is, the angle that adds
 * up to 360° with the original angle
 *
 * The angle must be larger than 180° and smaller than 360°. The given values
 * are overridden.
 *
 * @param deg 180..359 (not 180 if min==0 and sec==0)
 * @param min 0..59
 * @param sec 0..59
 */
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



/**
 * Normalizes the longitude
 *
 * A normalized longitude is in the range -179°59'59" to +180°0'0". The
 * normalized longitude of 0°0'0" has a positive sign.
 *
 * @return a new Longitude representing the same longitude, but normalized
 */
Longitude Longitude::normalized () const
{
	// TODO this needs a unit test

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

	if (deg==0 && min==0 && sec==0)
	{
		// Angles of exactly 0° are returned with a positive sign
		return Longitude (0, 0, 0, true);
	}
	else if (deg<180)
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
		// Other angles are complemented and the sign is inverted
		complementAngle (deg, min, sec);
		return Longitude (deg, min, sec, !positive);
	}
}

/**
 * Converts the longitude to a double value, representing the degrees
 *
 * @return a double representation of this longitude
 */
double Longitude::toDegrees () const
{
	double value = (double)degrees + minutes/(double)60 + seconds/(double)3600;

	if (positive)
		return value;
	else
		return -value;
}


/**
 * Creates a longitude from a double
 *
 * @return a new Longitude
 */
Longitude Longitude::fromDegrees (double deg)
{
	Longitude longitude;

	longitude.positive=(deg>0);

	longitude.degrees=(int)(deg);

	deg-=longitude.degrees;
	deg*=60;
	longitude.minutes=(int)(deg);

	deg-=longitude.minutes;
	deg*=60;
	longitude.seconds=(int)deg;

	return longitude;
}



/**
 *
 * @param other
 * @return
 */
double Longitude::minusDegrees (const Longitude &other) const
{
	return toDegrees ()-other.toDegrees ();
}
