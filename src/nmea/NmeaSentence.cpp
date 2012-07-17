#include "src/nmea/NmeaSentence.h"

#include <iostream>

#include "src/nmea/Nmea.h"
#include "src/util/qString.h"

/**
 * Creates an NMEA sentence
 *
 * @param line the line of data as received from the GPS device. Leading and
 *             trailing whitespace is ignored.
 * @param sentenceType the expected sentence type, without the leading dollar
 *                     sign; for example, "GPRMC". The sentence will be
 *                     considered invalid if the actual sentence type does not
 *                     match the expected sentence type.
 * @param numParts the expected minimum number of parts, including the sentence
 *                 type. The sentence will be considered invalid if it contains
 *                 less than the expected number of parts.
 */
NmeaSentence::NmeaSentence (const QString &line, const QString &sentenceType, int numParts):
	line (line.trimmed ()), valid (false)
{
	// Split the string
	// FIXME remove the checksum from the last part

	parts=line.split (',');

	// FIXME test
	if (!Nmea::verifyChecksum (line))
	{
		std::cout << "NMEA sentence checksum mismatch: " << line << std::endl;
		return;
	}

	// Make sure that it is not truncated
	// FIXME test
	// FIXME even if numParts is 0, make sure we have at least one part so we can access parts[0]
	if (parts.length () < numParts)
	{
		std::cout << "NMEA sentence truncated: " << line << std::endl;
		return;
	}

	// Make sure that the sentence type is correct
	// FIXME test
	if (parts[0]!="$"+sentenceType)
	{
		std::cout << "NMEA sentence type mismatch (expected " << sentenceType << "): " << std::endl;
		return;
	}

	valid=true;
}

NmeaSentence::~NmeaSentence ()
{
}

/**
 * Returns the original received line, as passed to the constructor
 */
QString NmeaSentence::getLine () const
{
	return line;
}

/**
 * Returns true if the sentence is considered valid
 */
bool NmeaSentence::isValid () const
{
	return valid;
}

/**
 * Returns a list of parts (separated by commas in the received line)
 *
 * This list includes the sentence type part (e. g. "$GPRMC") as item 0.
 */
QStringList NmeaSentence::getParts () const
{
	return parts;
}
