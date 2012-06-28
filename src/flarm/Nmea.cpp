#include <src/flarm/Nmea.h>

#include <QString>
#include <QStringList>

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
 * @param sentence
 * @return
 */
uint8_t Nmea::calculateChecksum (const QString &sentence)
{
	uchar checksum=0;

	for (int i=1, n=sentence.length (); i < n; ++i)
	{
		uint8_t character = (sentence[i]).toAscii();

		// Ignore the start sign
		if (character == '$')
			continue;

		// Stop at the end of the sentence
		if (character == '*') // End of sentence reached
			break;

		// Update the checksum
		checksum ^= character;
	}

	return checksum;
}

/**
 * Checks that the checksum recorded in the sentence matches the sentence
 *
 * @param sentence the sentence to test
 * @return true if the checksum is correct, false otherwise
 */
bool Nmea::verifyChecksum (const QString &sentence)
{
	// Split the sentence on an asterisk. The first part is the actual sentence,
	// the second part is the checksum.
	QStringList list = sentence.split ('*');

	if (list.length () > 1)
	{
		// Get the checksum from the sentence and compare it with the calculated
		// checksum
		uint8_t checksum = (uchar)list[1].trimmed ().toUShort (0, 16);
		return (checksum == calculateChecksum (list[0]));
	}
	else
	{
		// The sentence does not contain a checksum
		return false;
	}
}

Angle Nmea::parseLatitude  (const QString &value, const QString &sign)
{
	return parseAngle (value, sign, 2, "N", "S");
}

Angle Nmea::parseLongitude (const QString &value, const QString &sign)
{
	return parseAngle (value, sign, 3, "E", "W");
}

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
		return Angle (+magnitude);
	else if (sign==negativeSign)
		return Angle (-magnitude);
	else
		return Angle ();
}

/**
 *
 * @param value
 * @return the date in UTC (QDate does not know about time zones)
 */
QDate Nmea::parseDate (const QString &value)
{
	// Format of value is ddmmyy
	QDate date (QDate::fromString (value, "ddMMyy"));

	// The two-digit year is interpreted as 19xx. We have to add 100 years :-/.
	return date.addYears (100);
}

/**
 *
 * @param value
 * @return the time in UTC (QDate does not know about time zones)
 */
QTime Nmea::parseTime (const QString &value)
{
	// Format of value is hhmmss.xx
	QString hhmmss=value.split ('.')[0];
	return QTime::fromString (hhmmss, "hhmmss");
}

QDateTime Nmea::parseDateTime (const QString &dateValue, const QString &timeValue)
{
	return QDateTime (parseDate (dateValue), parseTime (timeValue), Qt::UTC);
}
