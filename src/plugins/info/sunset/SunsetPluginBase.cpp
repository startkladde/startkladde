/*
 * SunsetPluginBase.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetPluginBase.h"

#include <QDebug>
#include <QSettings>
#include <QRegExp>
#include <QFile>

#include "SunsetPluginSettingsPane.h"
#include "src/text.h"
#include "src/util/file.h"
#include "src/util/qString.h"
#include "src/util/time.h"


// ******************
// ** Construction **
// ******************

SunsetPluginBase::SunsetPluginBase (QString caption, bool enabled, const QString &filename):
	InfoPlugin (caption, enabled),
	filename (filename),
	longitudeCorrection (false),
	referenceLongitudeValid (false)
{
}

SunsetPluginBase::~SunsetPluginBase ()
{
}


// ************************
// ** InfoPlugin methods **
// ************************

PluginSettingsPane *SunsetPluginBase::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new SunsetPluginSettingsPane (this, parent);
}

void SunsetPluginBase::infoPluginReadSettings (const QSettings &settings)
{
	filename=settings.value ("filename", filename).toString ();
	bool longitudeOk=false;
	longitude=Longitude::fromString (settings.value ("longitude", "9 27 0").toString (), &longitudeOk);
	longitudeCorrection=longitudeOk && settings.value ("longitudeCorrection", false).toBool ();
}

void SunsetPluginBase::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("filename", filename);
	settings.setValue ("longitude", longitude.toString ());
	settings.setValue ("longitudeCorrection", longitudeCorrection);
}

QString SunsetPluginBase::configText () const
{
	if (longitudeCorrection)
		return QString ("%1, %2").arg (longitude.format (), filename);
	else
		return filename;
}

#define OUTPUT_AND_RETURN(text) do { outputText (utf8 (text)); return; } while (0)

/**
 * Reads the required data from the data file
 *
 * Resolves the file name. The following data is read:
 *   - the sunset for the current date
 *   - the reference longitude, if longitude correction is activated
 *
 * If the reading fails, the correspondig values are set to invalid
 * (invalid sunset to or referenceLongitudeOk=false) and an error message
 * is output.
 */
void SunsetPluginBase::start ()
{
	QString filename=getFilename ();

	resolvedFilename=QString ();
	rawSunset=correctedSunset=QTime ();
	referenceLongitudeValid=false;

	if (blank (filename)) OUTPUT_AND_RETURN ("Keine Datei angegeben");

	resolvedFilename=resolveFilename (filename);
	if (blank (resolvedFilename)) OUTPUT_AND_RETURN ("Datei nicht gefunden");
	if (!QFile::exists (resolvedFilename)) OUTPUT_AND_RETURN ("Datei existiert nicht");

	try
	{
		// The file is OK

		// Find the sunset for today
		QString sunsetString=readSunsetString (resolvedFilename);
		if (blank (sunsetString)) OUTPUT_AND_RETURN ("Zeit für aktuelles Datum nicht in Datendatei vorhanden");

		rawSunset=QTime::fromString (sunsetString, "hh:mm");
		if (!rawSunset.isValid ()) OUTPUT_AND_RETURN ("Ungültiges Zeitformat");

		if (longitudeCorrection)
		{
			referenceLongitude=readReferenceLongitude (resolvedFilename, &referenceLongitudeValid);
			if (!referenceLongitudeValid) OUTPUT_AND_RETURN ("Kein Bezugslängengrad in Datendatei gefunden");

			correctedSunset=localSunset (longitude, referenceLongitude, rawSunset);
		}
	}
	catch (FileOpenError &ex)
	{
		outputText ("Fehler: "+ex.errorString);
	}
}

/**
 * Discards the values read from the time
 */
void SunsetPluginBase::terminate ()
{
	rawSunset=QTime ();
	correctedSunset=QTime ();
}


// ******************
// ** File reading **
// ******************

/**
 * Reads the sunset from a file as a string
 *
 * The file must contain a line with the current date and the sunset time,
 * separated by whitespace. Example
 *   08-15  18:43
 *
 * If there is no entry for the current date in the file, an empty string is
 * returned.
 *
 * @param filename the file to read from
 * @return the sunset string
 * @throw FileOpenError if the file cannot be opened
 */
QString SunsetPluginBase::readSunsetString (const QString &filename)
{
	QString dateString=QDate::currentDate ().toString ("MM-dd");
	QRegExp regexp (QString ("^%1\\s*(\\S*)").arg (dateString));

	return findInFile (filename, regexp, 1);
}

/**
 * Reads the reference longitude from a file
 *
 * The file must contain a line with the following format:
 *   ReferenceLongitude: +9 27 00
 *
 * If there is no entry for the reference longitude in the file, an empty
 * string is returned.
 *
 * @param filename the file to read from
 * @param ok set to true on success or false if there was no reference
 *           longitude entry or it could not be parsed
 * @return the reference longitude
 * @throw FileOpenError if the file cannot be opened
 */
Longitude SunsetPluginBase::readReferenceLongitude (const QString &filename, bool *ok)
{
	QString refLon=findInFile (filename, QRegExp ("^ReferenceLongitude: (.*)"), 1);

	return Longitude::fromString (refLon, ok);
}

/**
 * Reads the source from a file
 *
 * The file must contain a line with the following format:
 *   Source: xyz
 *
 * If there is no entry for the source in the file, an empty
 * string is returned.
 *
 * @param filename the file to read from
 * @return the source
 * @throw FileOpenError if the file cannot be opened
 */
QString SunsetPluginBase::readSource (const QString &filename)
{
	QRegExp regexp ("^Source: (.*)");

	return findInFile (filename, regexp, 1);
}


// **********
// ** Misc **
// **********

/**
 * Returns the raw or corrected sunset time, depending on the longitude
 * correction setting
 *
 * @return the sunset time to use
 */
QTime SunsetPluginBase::getEffectiveSunset ()
{
	if (longitudeCorrection)
		return getCorrectedSunset ();
	else
		return getRawSunset ();
}
