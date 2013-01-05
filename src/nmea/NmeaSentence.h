#ifndef NMEASENTENCE_H_
#define NMEASENTENCE_H_

#include <QString>
#include <QStringList>

/**
 * An NMEA sentence
 *
 * The primary use of this class is to be inherited by a concrete NMEA sentence
 * class (e. g. GprmcSentence). It can also be used directly, although that's
 * probably not very useful.
 *
 * Implementation notes: after calling the constructor, call isValid() to make
 * sure that the sentence is valid. If isValid() returns true, use getParts() to
 * access the individual parts of the sentence. The parts list is guaranteed to
 * contain at least the specified number of parts in this case. If isValid()
 * returns false, the parts list may shorter. In this case, you should abort.
 */
class NmeaSentence
{
	public:
		NmeaSentence (const QString &line, const QString &sentenceType, int numParts);
		virtual ~NmeaSentence ();

		QString getLine () const;
		QStringList getParts () const;
		bool isValid () const;

	private:
		QString line;
		QStringList parts;
		bool valid;
};

#endif

