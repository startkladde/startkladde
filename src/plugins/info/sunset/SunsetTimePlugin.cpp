/*
 * SunsetTimePlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetTimePlugin.h"

#include <QFile>

#include "src/plugin/info/InfoPluginFactory.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/text.h"

REGISTER_INFO_PLUGIN (SunsetTimePlugin)

SunsetTimePlugin::SunsetTimePlugin (QString caption, bool enabled, const QString &filename):
	SunsetPluginBase (caption, enabled, filename)
{
}

SunsetTimePlugin::~SunsetTimePlugin ()
{
}

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

void SunsetTimePlugin::start ()
{
	SunsetPluginBase::start ();

	if (sunsetValid)
		outputText (sunset);
}
