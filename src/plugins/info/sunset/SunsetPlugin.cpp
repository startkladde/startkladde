/*
 * SunsetPlugin.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetPlugin.h"

#include <QDebug>
#include <QSettings>
#include <QRegExp>
#include <QFile>

#include "src/plugin/info/InfoPluginFactory.h"
#include "SunsetPluginSettingsPane.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/file.h"

REGISTER_INFO_PLUGIN (SunsetPlugin)

SunsetPlugin::SunsetPlugin (QString caption, bool enabled, const QString &filename):
	InfoPlugin (caption, enabled),
	filename (filename)
{
}

SunsetPlugin::~SunsetPlugin ()
{
}

QString SunsetPlugin::getId () const
{
	return "sunset";
}

QString SunsetPlugin::getName () const
{
	return "Sunset";
}

QString SunsetPlugin::getDescription () const
{
	return utf8 ("Zeigt die Sonnenuntergangszeit an");
}

PluginSettingsPane *SunsetPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new SunsetPluginSettingsPane (this, parent);
}

void SunsetPlugin::infoPluginReadSettings (const QSettings &settings)
{
	filename=settings.value ("filename", filename).toString ();
}

void SunsetPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("filename", filename);
}

void SunsetPlugin::start ()
{
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
				QString sunset=findSunset ();
				if (blank (sunset))
					outputText ("Datum nicht in Datendatei vorhanden");
				else
					outputText (sunset);
			}
		}
	}
	catch (FileOpenError &ex)
	{
		outputText ("Fehler: "+ex.errorString);
	}
}

void SunsetPlugin::terminate ()
{
}

QString SunsetPlugin::configText () const
{
	return filename;
}

/**
 * Throws FileOpenError
 *
 * @return
 */
QString SunsetPlugin::findSunset ()
{
	QString dateString=QDate::currentDate ().toString ("MM-dd");

	QRegExp regexp (QString ("^%1\\s*(\\S*)").arg (dateString));
	if (findInFile (resolveFilename (filename), regexp))
		return regexp.cap (1);
	else
		return QString ();
}
