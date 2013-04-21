#include "NmeaDecoder.h"

#include "src/io/dataStream/DataStream.h"
#include "src/nmea/Nmea.h"
#include "src/nmea/GprmcSentence.h"
#include "src/nmea/PflaaSentence.h"

NmeaDecoder::NmeaDecoder (QObject *parent): QObject (parent)
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
	if (Nmea::isType ("GPRMC", line))
	{
		GprmcSentence sentence (line);
		if (sentence.isValid ()) emit gprmcSentence (sentence);
	}
	else if (Nmea::isType ("PFLAA", line))
	{
		PflaaSentence sentence (line);
		if (sentence.isValid ()) emit pflaaSentence (sentence);
	}
}
