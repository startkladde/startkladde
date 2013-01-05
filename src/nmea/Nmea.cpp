#include "Nmea.h"

#include <QString>
#include <QStringList>

/**
 * Private constructor, this class cannot be created
 */
Nmea::Nmea ()
{
}

Nmea::~Nmea ()
{
}


/**
 * Calculates the checksum for an NMEA sentence
 *
 * NMEA-0183 Standard:
 *   The optional checksum field consists of a "*" and two hex digits
 *   representing the exclusive OR of all characters between, but not including,
 *   the "$" and "*". A checksum is required on some sentences.
 *
 * The checksum is calculated for the whole argument. The argument must
 * therefore contain everything between, but not including, the leading "$" and
 * the "*" separating the sentence from the checksum.
 *
 * The checksum is returned as numeric value and must be converted to a
 * hexadecimal ASCII representation.
 */
uint8_t Nmea::calculateChecksum (const QString &data)
{
	uchar checksum=0;

	for (int i=0, n=data.length (); i<n; ++i)
	{
		uint8_t character = data[i].toAscii ();
		checksum ^= character;
	}

	return checksum;
}

/**
 * Checks if the string passed is a correct NMEA sentence
 *
 * The argument may contain leading or trailing whitespace.
 */
bool Nmea::sentenceValid (const QString &sentence)
{
	// $GPRMC,103400.00,A,5256.58562,N,01247.34325,E,0.002,,100911,,,A*77

	// Remove whitespace from the beginning and end of the string
	QString s=sentence.trimmed ();
	int l=s.length ();

	// The string must be long enough to contain the $ and the checksum.
	if (s.length ()<4) return false;

	// The string must begin with a $ and contain a * at the third place from
	// the end
	if (s[0]  .toAscii ()!='$') return false;
	if (s[l-3].toAscii ()!='*') return false;

	// Extract the data part and the checksum
	QString data    =s.mid (1  , l-4);
	QString checksum=s.mid (l-2, 2  );

	// The extracted checksum must match the calculated checksum
	uint8_t calculatedChecksum=calculateChecksum (data);
	uint8_t extractedChecksum=checksum.toUShort (NULL, 16);
	qDebug () << calculatedChecksum << extractedChecksum;
	if (calculatedChecksum!=extractedChecksum) return false;

	// Everything seems to be in order
	return true;
}

/**
 * Returns the parts of a sentence, including the name part (e. g. "GPRMC")
 *
 * The result is undefined if the sentence is not valid (as checked by
 * sentenceValid).
 */
QStringList Nmea::sentenceParts (const QString &sentence)
{
	QString s=sentence.trimmed ();
	int l=s.length ();

	return s.mid (1, l-4).split (',');
}

/**
 * Parses a latitude value
 *
 * A latitude value has two digits before the decimal point and a N/S sign
 * designator
 *
 * @see parseAngle
 */
Angle Nmea::parseLatitude  (const QString &value, const QString &sign)
{
	return parseAngle (value, sign, 2, "N", "S");
}

/**
 * Parses a latitude value
 *
 * A latitude value has three digits before the decimal point and a E/W sign
 * designator
 *
 * @see parseAngle
 */
Angle Nmea::parseLongitude (const QString &value, const QString &sign)
{
	return parseAngle (value, sign, 3, "E", "W");
}

/**
 * Parses an angle in NMEA format
 *
 * This is a private helper function for the frontend functions parseLatitude
 * and parseLongitude.
 *
 * If the sign matches neither the positive nor the negative sign, an invalid
 * angle is returned.
 *
 * @param value the string value to parse, e. g. 01122.30000 for 11°22.3'
 * @param sign a string (typically single-character) that designates the sign of
 *             the value
 * @param degreeDigits the number of digits to use for the degree value, 3 in
 *                     the example
 * @param positiveSign the sign string that designates a positive sign
 * @param negativeSign the sign string that designates a negative sign
 * @return
 */
Angle Nmea::parseAngle (const QString &value, const QString sign, int degreeDigits, const QString &positiveSign, const QString &negativeSign)
{
	// Format of value is DDMM.MMMM

	bool ok=false;

	// If the string is too short, return an invalid angle.
	if (value.length ()<degreeDigits+1) return Angle ();

	// Extract the degrees. If invalid, return an invalid angle.
	int degrees=value.left (degreeDigits).toInt (&ok);
	if (!ok) return Angle ();

	// Extract the minutes. If invalid, return an invalid angle.
	double minutes=value.mid (degreeDigits).toDouble (&ok);
	if (!ok) return Angle ();

	// Depending on the sign, return the value or an invalid value.
	double magnitude=degrees+minutes/60;
	if (sign==positiveSign)
		return Angle::fromDegrees (+magnitude);
	else if (sign==negativeSign)
		return Angle::fromDegrees (-magnitude);
	else
		return Angle ();
}

/**
 * Parses a date in NMEA format
 *
 * @param value the date value, e. g. 311210 for December 1, 2010
 * @return the date in the NMEA time zone (QDate does not know about time zones)
 */
QDate Nmea::parseDate (const QString &value)
{
	// Format of value is ddmmyy
	QDate date (QDate::fromString (value, "ddMMyy"));

	// The two-digit year is interpreted as 19xx. We have to add 100 years :-/.
	return date.addYears (100);
}

/**
 * Parses a time in NMEA format
 *
 * @param value the time value, e. g. 142835 for 14:28:35
 * @return the time in the NMEA time zone (QDate does not know about time zones)
 */
QTime Nmea::parseTime (const QString &value)
{
	// Format of value is hhmmss.xx
	QString hhmmss=value.split ('.')[0];
	return QTime::fromString (hhmmss, "hhmmss");
}

/**
 * A convenience method for parsing both a date and a time
 *
 * Contrary to QDate and QTime, QDateTime does know about time zones. The date
 * and time values are assumed to be in UTC.
 */
QDateTime Nmea::parseDateTime (const QString &dateValue, const QString &timeValue)
{
	return QDateTime (parseDate (dateValue), parseTime (timeValue), Qt::UTC);
}
