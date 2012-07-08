#include "NmeaDecoder.h"

#include "src/io/dataStream/DataStream.h"

#include "src/nmea/GprmcSentence.h"
#include "src/nmea/PflaaSentence.h"

NmeaDecoder::NmeaDecoder ()
{
}

NmeaDecoder::~NmeaDecoder ()
{
}

/**
 * Decodes a line and emits a signal for recognized sentences
 *
 * @param line a complete line
 */
// FIXME with or without line terminator?
void NmeaDecoder::lineReceived (const QString &line)
{
	//	qDebug () << "NMEA decoder: process sentence " << line;

//	if (stream) *stream << line;

	// TODO also process GPGGA and GPGSA/GPGSV sentences
	if      (line.startsWith ("$GPRMC")) emit gprmcSentence (GprmcSentence (line));
	else if (line.startsWith ("$PFLAA")) emit pflaaSentence (PflaaSentence (line));
}
