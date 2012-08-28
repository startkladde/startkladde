#include <src/flarm/FlarmNetFile.h>

// According to XCSoar's FlarmNetReader.cpp, FlarmNet files are "ISO-Latin-1,
// which is kind of short-sighted"

// Example:
// 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4 4 4 5 5 5 5 5 5 5 5 5 5 6 6 6 6 6 6 6 6 6 6 7 7 7 7 7 7 7 7 7 7 8 8 8 8 8 8
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// 4646463031314653562d4765727374657474656e20202020202020454450542020202020202020202020202020202020526f62696e20445220343030202020202020202020442d45415246205246203132332e303030
// F F F 0 1 1 F S V - G e r s t e t t e n               E D P T                                   R o b i n   D R   4 0 0                   D - E A R F   R F   1 2 3 . 0 0 0
// FlarmID-----Owner-------------------------------------                                          Type--------------------------------------Registration--CS----Frequency-----

QString FlarmNetFile::decodeLine (const QString &rawLine)
{
	QString result;

	int lineLength=rawLine.length ();

	// Iterate over the characters of the line in increments of 2
	for (int i=0; i<lineLength-1; i+=2)
	{
		QString substring=rawLine.mid (i, 2);

		bool ok=false;
		int character=substring.toInt (&ok, 16);

		if (ok)
			result.append (QChar (character));
	}

	return result;
}

/**
 * Creates a FlarmNet record from a decoded line and returns it as value
 *
 * If ok is not NULL, it will be set to true if the line is valid or false if
 * the line is invalid. The return value is undefined if the line is invalid.
 */
FlarmNetRecord FlarmNetFile::createRecord (const QString &decodedLine, bool *ok)
{
	FlarmNetRecord record;

	// Initialize the OK flag to false. It will be set to true when the line is
	// valid.
	bool isOk=false;

	if (decodedLine.length ()==86)
	{
		record.flarmId     =(decodedLine.mid ( 0,  6).trimmed ());
		record.owner       =(decodedLine.mid ( 6, 21).trimmed ());
		record.airfield    =(decodedLine.mid (27, 21).trimmed ());
		record.type        =(decodedLine.mid (48, 20).trimmed ());
		record.registration=(decodedLine.mid (69,  7).trimmed ());
		record.callsign    =(decodedLine.mid (76,  3).trimmed ());
		record.frequency   =(decodedLine.mid (79,  7).trimmed ());

		// Perform some basic validity checks and set the OK flag on success
		if (record.flarmId.length ()==6 && record.registration.length ()>0)
			isOk=true;
	}

	if (ok)
		(*ok)=isOk;

	return record;
}

QList<FlarmNetRecord> FlarmNetFile::createRecordsFromFile (const QString &data, int *numGood, int *numBad)
{
	QList<FlarmNetRecord> result;

	// Initialize the counters for good and bad lines
	int good=0;
	int bad=0;

	// Split the data into lines
	QStringList rawLines=data.split (QRegExp ("[\r\n]"), QString::SkipEmptyParts);

	// The first line is some kind of header, ignore it
	rawLines.removeFirst ();

	// Iterate over the (remaining) lines
	foreach (const QString &rawLine, rawLines)
	{
		// Decode the line
		QString decodedLine=decodeLine (rawLine);

		// Create a record from the line
		bool ok=false;
		FlarmNetRecord record=createRecord (decodedLine, &ok);

		if (ok)
		{
			++good;
			result.append (record);
		}
		else
		{
			++bad;
		}
	}

	// Write back the good and bad counts
	if (numGood) (*numGood)=good;
	if (numBad ) (*numBad )=bad ;

	return result;
}
