#ifndef FLARMNETFILE_H_
#define FLARMNETFILE_H_

#include <QString>

#include "src/flarm/flarmNet/FlarmNetRecord.h"

/**
 * Some helper methods for handling the FlarmNet file format
 */
class FlarmNetFile
{
	public:
		static QString decodeLine (const QString &rawLine);
		static FlarmNetRecord createRecord (const QString &decodedLine, bool *ok);
		static QList<FlarmNetRecord> createRecordsFromFile (const QString &data, int *numGood, int *numBad);
};

#endif
