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

// FIXME use longitude


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

void SunsetPluginBase::terminate ()
{
	rawSunset=QTime ();
	correctedSunset=QTime ();
}


// ******************
// ** File reading **
// ******************

/**
 * Throws FileOpenError
 */
QString SunsetPluginBase::readSunsetString (const QString &filename)
{
	QString dateString=QDate::currentDate ().toString ("MM-dd");
	QRegExp regexp (QString ("^%1\\s*(\\S*)").arg (dateString));

	return findInFile (filename, regexp, 1);
}

/**
 * Throws FileOpenError
 */
Longitude SunsetPluginBase::readReferenceLongitude (const QString &filename, bool *ok)
{
	QString refLon=findInFile (filename, QRegExp ("^ReferenceLongitude: (.*)"), 1);

	return Longitude::fromString (refLon, ok);
}

/**
 * Throws FileOpenError
 */
QString SunsetPluginBase::readSource (const QString &filename)
{
	QRegExp regexp ("^Source: (.*)");

	return findInFile (filename, regexp, 1);
}


// **********
// ** Misc **
// **********

QTime SunsetPluginBase::getEffectiveSunset ()
{
	if (longitudeCorrection)
		return getCorrectedSunset ();
	else
		return getRawSunset ();
}
