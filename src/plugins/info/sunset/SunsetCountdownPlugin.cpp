/*
 * SunsetCountdownPlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetCountdownPlugin.h"

#include <cstdlib>

#include <QFile>

#include "src/plugin/factory/PluginFactory.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/util/time.h"
#include "src/text.h"

REGISTER_PLUGIN (InfoPlugin, SunsetCountdownPlugin)
SK_PLUGIN_DEFINITION (SunsetCountdownPlugin, "{9735dfd7-ddfd-476c-affd-edbf196e0279}", "Sunset-Countdown", utf8 ("Zeigt die verbleibende Zeit bis Sonnenuntergang an"))

// ******************
// ** Construction **
// ******************

SunsetCountdownPlugin::SunsetCountdownPlugin (QString caption, bool enabled, const QString &filename):
	SunsetPluginBase (caption, enabled, filename)
{
}

SunsetCountdownPlugin::~SunsetCountdownPlugin ()
{
}


// ********************
// ** Plugin methods **
// ********************

/**
 * Calls SunsetPluginBase::start and update
 */
void SunsetCountdownPlugin::start ()
{
	SunsetPluginBase::start ();
	update ();
}

void SunsetCountdownPlugin::minuteChanged ()
{
	update ();
}


// *******************
// ** Functionality **
// *******************

/**
 * Outputs the time until sunset, in red color if negative
 */
void SunsetCountdownPlugin::update ()
{
	QTime sunsetTime=nullSeconds (getEffectiveSunset ());
	if (!sunsetTime.isValid ()) return;

	QTime currentTime=nullSeconds (currentTimeUtc ());

	int seconds=currentTime.secsTo (sunsetTime);

	QString color=(seconds<0)?"#FF0000":"#000000";
	QString duration=formatDuration (seconds, false);
	QString output=QString ("<font color=\"%1\">%2</font>").arg (color, duration);

	outputText (output, Qt::RichText);
}
