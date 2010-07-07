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
}

void SunsetPluginBase::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("filename", filename);
}

QString SunsetPluginBase::configText () const
{
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
