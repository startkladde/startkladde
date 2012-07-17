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
 * @param line a complete line, with or without line terminator
 */
void NmeaDecoder::lineReceived (const QString &line)
{
	// FIXME if not valid (checksum missing or truncated), don't emit
	if      (line.startsWith ("$GPRMC")) emit gprmcSentence (GprmcSentence (line));
	else if (line.startsWith ("$PFLAA")) emit pflaaSentence (PflaaSentence (line));
}
