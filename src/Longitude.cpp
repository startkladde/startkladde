/*
 * Longitude.cpp
 *
 *  Created on: 13.07.2010
 *      Author: Martin Herrmann
 */

#include "Longitude.h"

#include <cstdlib>
#include <cmath>

#include <QRegExp>
#include <QDebug>

#include "src/util/qString.h"


// ******************
// ** Construction **
// ******************

/**
 * Creates an invalid longitude
 */
Longitude::Longitude ():
	value (0), valid (false)
{
}

/**
 * Creates a longitude with a given value
 *
 * @param value the longitude value in degrees
 */
Longitude::Longitude (double value):
	value (value), valid (true)
{
}

/**
 * Creates a longitude with a value given as degrees, minutes and seconds and
 * a sign
 */
Longitude::Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive):
	valid (true)
{
	value = degrees + minutes/(double)60 + seconds/(double)3600;
	if (!positive) value=-value;
}

Longitude::~Longitude ()
{
}


// *********************
// ** Data processing **
// *********************

/**
 * Normalizes the longitude
 *
 * A normalized longitude is larger than -180° and less than or equal to +180°.
 *
 * @return a new Longitude representing the same longitude, but normalized
 */
Longitude Longitude::normalized () const
{
	double newValue=fmod (value, 360);

	if (newValue>180)
		newValue-=360;
	else if (newValue<=-180)
		newValue+=360;

	return Longitude (newValue);
}


// ****************
// ** Conversion **
// ****************

/**
 * Converts the longitude to a degrees/minutes/seconds representation with
 * minutes and seconds in the 0..59 range. The seconds are rounded
 * mathematically, rounding towards higher numbers for 0.5 seconds.
 *
 * No normalization is performed. Note that it is possible to get -180°0'0"
 * even with a normalized value.
 *
 * @param degrees the degrees are written here
 * @param minutes the minutes are written here
 * @param seconds the seconds are written here
 * @param positive set to true if the value is greater than or equal to 0,
 *                 false else
 * @param remainder the amount the returned value is too small
 */
void Longitude::toDms (uint &degrees, uint &minutes, uint &seconds, bool &positive, double &remainder) const
{
	// Calculate the value in seconds
	double valueSeconds=value*3600;

	// Round the seconds mathematically, rounding towards higher numbers for
	// 0.5 seconds
	int rounded=floor (valueSeconds+0.5);

	// Determine the rounding remainder
	remainder=(valueSeconds-rounded)/3600;

	// Note and remove the sign of the rounded value
	positive=(rounded>=0);
	rounded=abs (rounded);

	// Determine the seconds
	seconds=rounded%60;

	// Determine the minutes
	rounded=rounded/60;
	minutes=rounded%60;

	// Determine the degrees
	rounded=rounded/60;
	degrees=rounded;
}

void Longitude::toDms (uint &degrees, uint &minutes, uint &seconds, bool &positive) const
{
	double remainder;
	toDms (degrees, minutes, seconds, positive, remainder);
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
	unsigned int degrees, minutes, seconds;
	bool positive;
	toDms (degrees, minutes, seconds, positive);

	return QString (utf8 ("%1° %2' %3\" %4"))
		.arg (abs (degrees))
		.arg (minutes)
		.arg (seconds)
		.arg (positive?positiveString:negativeString);
}

/**
 * Creates a longitude by parsing a string
 *
 * Supported formats:
 *   -     dd mm ss       or       dd° mm' ss"
 *   - +|- dd mm ss       or   +|- dd° mm' ss"
 *   -     dd mm ss E|W   or       dd° mm' ss" E|W
 *
 * If parsing fails, an invalid Longitude is returned.
 *
 * @param string the string to parse. Leading and trailing whitespace is
 *        ignored.
 * @return the parse longitude, or an invalid longitude if the parsing failed
 */
Longitude Longitude::parse (const QString &string)
{
	static const Longitude invalid;

	QString s=string.trimmed ();
	if (s.isEmpty ()) return invalid;

	QRegExp re;

	// dd mm ss
	// dd° mm' ss"
	re=QRegExp (utf8 ("^(\\d+)°?\\s+(\\d+)'?\\s+(\\d+)\"?$"));
	if (s.contains (re))
		return parse (re, 1, 2, 3, true);

	// +|- d m s
	// +|- d° m' s"
	re=QRegExp (utf8 ("^([+-])\\s*(\\d+)°?\\s+(\\d+)'?\\s+(\\d+)\"?$"));
	if (s.contains (re))
		return parse (re, 2, 3, 4, re.cap (1)!="-");

	// dd mm ss E|W
	// dd° mm' ss" E|W
	re=QRegExp (utf8 ("^(\\d+)°?\\s+(\\d+)'?\\s+(\\d+)\"?\\s*([EW])$"), Qt::CaseInsensitive);
	if (s.contains (re))
		return parse (re, 1, 2, 3, re.cap (4)!="W" && re.cap (4)!="w");

	return invalid;
}

/**
 * Creates a longitude by parsing strings for degrees, minutes and seconds
 *
 * This is a helper method for the parse (const QString &) method.
 */
Longitude Longitude::parse (const QString &degrees, const QString &minutes, const QString &seconds, bool positive)
{
	bool numOk=false;

	int deg=degrees.toUInt (&numOk); if (!numOk) return Longitude ();
	int min=minutes.toUInt (&numOk); if (!numOk) return Longitude ();
	int sec=seconds.toUInt (&numOk); if (!numOk) return Longitude ();

	return Longitude (deg, min, sec, positive);
}

/**
 * Creates a longitude by parsing captured text of a regular expression for
 * degrees, minutes and seconds
 *
 * This is a helper method for the parse (const QString &) method.
 */
Longitude Longitude::parse (const QRegExp &re, int degreesCap, int minutesCap, int secondsCap, bool positive)
{
	return parse (re.cap (degreesCap), re.cap (minutesCap), re.cap (secondsCap), positive);
}
