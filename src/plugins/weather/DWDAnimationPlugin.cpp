/*
 * DWDAnimationPlugin.cpp
 *
 *  Created on: 25.07.2010
 *      Author: martin
 */

#include "DWDAnimationPlugin.h"

#include <QMovie>
#include <QRegExp>

#include "src/plugin/factory/PluginFactory.h"
#include "src/net/Downloader.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/text.h"
#include "src/graphics/SkMovie.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (WeatherPlugin, DWDAnimationPlugin)
SK_PLUGIN_DEFINITION (
	DWDAnimationPlugin,
	notr ("{f3b7c9b2-455f-459f-b636-02b2b9a78b7b}"),
	DWDAnimationPlugin::tr ("DWD rain radar (animation)"),
	DWDAnimationPlugin::tr ("Displays a rain radar animation from dwd.de"))

enum State { stateNavigationPage, stateRadarPage, stateRadarImage };

const QString movieUrl (notr ("http://www.dwd.de/wundk/radar/Radarfilm_WEB_DL.gif"));

DWDAnimationPlugin::DWDAnimationPlugin ():
	downloader (new Downloader (this))
{
	downloader->connectSignals (this);
}

DWDAnimationPlugin::~DWDAnimationPlugin ()
{
}

void DWDAnimationPlugin::refresh ()
{
	outputText (tr ("Downloading radar animation..."));
	downloader->startDownload (0, movieUrl);
}

void DWDAnimationPlugin::abort ()
{
	downloader->abort ();
}

void DWDAnimationPlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
	outputText (tr ("Saving radar animation"));
	SkMovie movie (reply);
	if (!movie.getMovie ()->isValid ()) OUTPUT_AND_RETURN (tr ("Error reading the animation"));
	movie.getMovie ()->setSpeed (200);
	outputMovie (movie);
}

void DWDAnimationPlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	if (code==QNetworkReply::ContentNotFoundError)
	{
		outputText (tr ("Error: radar animation not found (404)"));
	}
	else
	{
		outputText (reply->errorString ());
	}
}


void DWDAnimationPlugin::languageChanged ()
{
	// Nothing to do
}
