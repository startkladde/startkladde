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
SK_PLUGIN_DEFINITION (SunsetTimePlugin, "{1998d604-e819-4aee-af3d-f0c5cee4c508}", "Sunset", utf8 ("Zeigt die Sonnenuntergangszeit an"))

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

const QString timeFormat="hh:mm";

/**
 * Calls SunsetPluginBase::start and outputs the sunset time in local time or
 * UTC, depending on the configuration
 */
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
