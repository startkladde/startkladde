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

// FIXME use longitude

SunsetPluginBase::SunsetPluginBase (QString caption, bool enabled, const QString &filename):
	InfoPlugin (caption, enabled),
	sunsetValid (false),
	filename (filename)
{
}

SunsetPluginBase::~SunsetPluginBase ()
{
}

PluginSettingsPane *SunsetPluginBase::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new SunsetPluginSettingsPane (this, parent);
}

void SunsetPluginBase::infoPluginReadSettings (const QSettings &settings)
{
	filename=settings.value ("filename", filename).toString ();
	bool longitudeOk=false;
	longitude=Longitude::fromString (settings.value ("longitude").toString (), &longitudeOk);
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
			QString fn=resolveFilename (filename);
			if (blank (fn))
				outputText ("Datei nicht gefunden");
			else if (!QFile::exists (fn))
				outputText ("Datei nicht gefunden");
			else
			{
				QString ss=findSunset ();
				if (blank (ss))
					outputText ("Datum nicht in Datendatei vorhanden");
				else
				{
					sunset=ss;
					sunsetValid=true;
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
	sunset=QString ();
	sunsetValid=false;
}


/**
 * Throws FileOpenError
 *
 * @return
 */
QString SunsetPluginBase::findSunset ()
{
	QString dateString=QDate::currentDate ().toString ("MM-dd");

	QRegExp regexp (QString ("^%1\\s*(\\S*)").arg (dateString));
	if (findInFile (resolveFilename (filename), regexp))
		return regexp.cap (1);
	else
		return QString ();
}

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

QString SunsetPluginBase::findSource (const QString &filename)
{
	return findInFile (filename, QRegExp ("^Source: (.*)"), 1);
}
