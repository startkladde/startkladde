#ifndef NMEADECODER_H_
#define NMEADECODER_H_

#include <QObject>

class DataStream;

class GprmcSentence;
class PflaaSentence;

/**
 * Decodes lines and emits NMEA sentences
 *
 * Connect a signal to lineReceived or call it directly. If the line is a
 * recognized NMEA sentence, it will be decoded and the respective signal
 * (xxxxxSentence) emitted. If the line is not an NMEA sentence, or not a
 * recognized NMEA sentence, it will be ignored.
 */
// FIXME clarify what happens when the checksum is invalid
class NmeaDecoder: public QObject
{
		Q_OBJECT

	public:
		NmeaDecoder ();
		virtual ~NmeaDecoder ();

	public slots:
		void lineReceived (const QString &line);

	signals:
		void gprmcSentence (const GprmcSentence &sentence);
		void pflaaSentence (const PflaaSentence &sentence);
};

#endif
