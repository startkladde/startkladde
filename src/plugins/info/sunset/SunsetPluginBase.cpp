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

void SunsetPluginBase::start ()
{
	QString filename=getFilename ();

	try
	{
		if (blank (filename))
			outputText ("Keine Datei angegeben");
		else
		{
			resolvedFilename=resolveFilename (filename);
			if (blank (resolvedFilename))
				outputText ("Datei nicht gefunden");
			else if (!QFile::exists (resolvedFilename))
				outputText ("Datei nicht gefunden");
			else
			{
				// The file is OK

				// Find the sunset for today
				QString ss=findSunsetString ();
				if (blank (ss))
					outputText ("Datum nicht in Datendatei vorhanden");
				else
					rawSunset=QTime::fromString (ss, "hh:mm");

				if (longitudeCorrection)
				{
					referenceLongitude=readReferenceLongitude (&referenceLongitudeValid);

					if (!referenceLongitudeValid)
					{
						outputText (utf8 ("Kein Bezugslängengrad in Datendatei gefunden"));
					}
					else
					{
						double relativeLongitude=longitude.minusDegrees (referenceLongitude);
						int timeDifferenceSeconds=(24*3600)*relativeLongitude/360;


						// Bigger (easterner) longitude means earlier sunset
						correctedSunset=rawSunset.addSecs (-timeDifferenceSeconds);
					}
				}
			}
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
QString SunsetPluginBase::findSunsetString ()
{
	QString dateString=QDate::currentDate ().toString ("MM-dd");

	QRegExp regexp (QString ("^%1\\s*(\\S*)").arg (dateString));
	if (findInFile (resolvedFilename, regexp))
		return regexp.cap (1);
	else
		return QString ();
}

/**
 * Throws FileOpenError
 */
Longitude SunsetPluginBase::findReferenceLongitude (const QString &filename, bool *ok)
{
	QString refLon=findInFile (filename, QRegExp ("^ReferenceLongitude: (.*)"), 1);

	if (refLon.isEmpty ())
	{
		if (ok) *ok=false;
		return Longitude ();
	}

	return Longitude::fromString (refLon, ok);
}

/**
 * Throws FileOpenError
 */
QString SunsetPluginBase::findSource (const QString &filename)
{
	return findInFile (filename, QRegExp ("^Source: (.*)"), 1);
}

/**
 * Throws FileOpenError
 */
QString SunsetPluginBase::readSource ()
{
	return findSource (resolvedFilename);
}

/**
 * Throws FileOpenError
 */
Longitude SunsetPluginBase::readReferenceLongitude (bool *ok)
{
	return findReferenceLongitude (resolvedFilename, ok);
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
