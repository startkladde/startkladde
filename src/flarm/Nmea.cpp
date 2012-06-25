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
