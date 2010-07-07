/*
 * SunsetCountdownPlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetCountdownPlugin.h"

#include <cstdlib>

#include <QFile>

#include "src/plugin/info/InfoPluginFactory.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/text.h"

REGISTER_INFO_PLUGIN (SunsetCountdownPlugin)

SunsetCountdownPlugin::SunsetCountdownPlugin (QString caption, bool enabled, const QString &filename):
	SunsetPluginBase (caption, enabled, filename)
{
}

SunsetCountdownPlugin::~SunsetCountdownPlugin ()
{
}

QString SunsetCountdownPlugin::getId () const
{
	return "sunset_countdown";
}

QString SunsetCountdownPlugin::getName () const
{
	return "Sunset-Countdown";
}

QString SunsetCountdownPlugin::getDescription () const
{
	return utf8 ("Zeigt die verbleibende Zeit bis Sonnenuntergang an");
}


void SunsetCountdownPlugin::start ()
{
	SunsetPluginBase::start ();

	if (sunsetValid)
		sunsetTime=QTime::fromString (sunset, "hh:mm");

	update ();
}

void SunsetCountdownPlugin::minuteChanged ()
{
	update ();
}

void SunsetCountdownPlugin::update ()
{
	if (sunsetTime.isValid ())
	{
		QTime currentTime=QDateTime::currentDateTime ().toUTC ().time ();
		currentTime.setHMS (currentTime.hour (), currentTime.minute (), 0);
		int seconds=currentTime.secsTo (sunsetTime);

		bool negative=(seconds<0);
		seconds=std::abs (seconds);

		uint minutes=seconds/60;
		seconds=seconds%60;

		uint hours=minutes/60;
		minutes=minutes%60;

		QString formatedDt=QString ("%1:%2")
			.arg (hours, 2, 10, QChar ('0'))
			.arg (minutes, 2, 10, QChar ('0'));

		QString output;
		if (negative)
			output=QString ("<font color=\"#FF0000\">-%1</font>").arg (formatedDt);
		else
			output=QString ("<font color=\"#000000\">%1</font>").arg (formatedDt);

		outputText (output, Qt::RichText);
	}
}
