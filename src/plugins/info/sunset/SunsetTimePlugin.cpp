/*
 * SunsetTimePlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetTimePlugin.h"

#include <QFile>
#include <QSettings>

#include "src/plugin/info/InfoPluginFactory.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/util/time.h"
#include "src/text.h"

REGISTER_INFO_PLUGIN (SunsetTimePlugin)


// ******************
// ** Construction **
// ******************

SunsetTimePlugin::SunsetTimePlugin (QString caption, bool enabled, const QString &filename):
	SunsetPluginBase (caption, enabled, filename)
{
}

SunsetTimePlugin::~SunsetTimePlugin ()
{
}


// ********************
// ** Plugin methods **
// ********************

QString SunsetTimePlugin::getId () const
{
	return "sunset";
}

QString SunsetTimePlugin::getName () const
{
	return "Sunset";
}

QString SunsetTimePlugin::getDescription () const
{
	return utf8 ("Zeigt die Sonnenuntergangszeit an");
}

const QString timeFormat="hh:mm";

void SunsetTimePlugin::start ()
{
	SunsetPluginBase::start ();

	QTime sunsetTime=getEffectiveSunset ();
	if (!sunsetTime.isValid ()) return;

	if (displayUtc)
		outputText (sunsetTime.toString (timeFormat)+" UTC");
	else
		outputText (utcToLocal (sunsetTime).toString (timeFormat));
}


// ************************
// ** InfoPlugin methods **
// ************************

void SunsetTimePlugin::infoPluginReadSettings (const QSettings &settings)
{
	SunsetPluginBase::infoPluginReadSettings (settings);

	displayUtc=settings.value ("displayUtc", true).toBool ();
}

void SunsetTimePlugin::infoPluginWriteSettings (QSettings &settings)
{
	SunsetPluginBase::infoPluginWriteSettings (settings);

	settings.setValue ("displayUtc", displayUtc);
}
