/*
 * WetterOnlineAnimationPlugin.cpp
 *
 *  Created on: 25.07.2010
 *      Author: martin
 */

#include "WetterOnlineAnimationPlugin.h"

#include <QMovie>
#include <QRegExp>
#include <QTemporaryFile>
#include <QSharedPointer>

#include "src/plugin/factory/PluginFactory.h"
#include "src/net/Downloader.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/text.h"

REGISTER_WEATHER_PLUGIN (WetterOnlineAnimationPlugin)
SK_PLUGIN_DEFINITION (WetterOnlineAnimationPlugin, "{f3b7c9b2-455f-459f-b636-02b2b9a78b7b}", "Wetter Online (Animation)", "Zeigt eine Regenradar-Animation von wetteronline.de an")

enum State { stateNavigationPage, stateRadarPage, stateRadarImage };

const QString navigationPageUrl ("http://www.wetteronline.de/include/radar_dldl_00_dwddgf.htm");

// FIXME "Operation canceled" output when network is disconnected (see WetterOnlineImagePlugin)

WetterOnlineAnimationPlugin::WetterOnlineAnimationPlugin ():
	downloader (new Downloader (this))
{
	downloader->connectSignals (this);
}

WetterOnlineAnimationPlugin::~WetterOnlineAnimationPlugin ()
{
}

void WetterOnlineAnimationPlugin::refresh ()
{
	outputText (utf8 ("Radarfilm herunterladen (1)..."));
	downloader->startDownload (stateNavigationPage, navigationPageUrl);
}

void WetterOnlineAnimationPlugin::abort ()
{
	downloader->abort ();
}

void WetterOnlineAnimationPlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
	switch ((State)state)
	{
		case stateNavigationPage:
		{
			QString radarPagePath=findInIoDevice (*reply, QRegExp ("a href.*a href=\"\\/([^\"]*)\".*Loop 3 Stunden"), 1);
			if (isBlank (radarPagePath)) OUTPUT_AND_RETURN ("Fehler: auf der Navigationsseite wurde kein Link zur Animation gefunden");
			QString radarPageUrl=QString ("http://www.wetteronline.de/%1").arg (radarPagePath);
			downloader->startDownload (stateRadarPage, radarPageUrl);
			outputText (utf8 ("Radarbild herunterladen (2)..."));
		} break;
		case stateRadarPage:
		{
			QString radarImagePath=findInIoDevice (*reply, QRegExp ("(daten\\/radar[^\"]*)\""), 1);
			if (isBlank (radarImagePath)) OUTPUT_AND_RETURN ("Fehler: auf der Wetterseite wurde keine Animation gefunden");
			QString radarImageUrl=QString ("http://www.wetteronline.de/%1").arg (radarImagePath);
			downloader->startDownload (stateRadarImage, radarImageUrl);
			outputText (utf8 ("Radarbild herunterladen (3)..."));
		} break;
		case stateRadarImage:
		{
			outputText (utf8 ("Radarbild speichern"));

			QSharedPointer<QTemporaryFile> file (new QTemporaryFile ());
			if (!file->open ()) OUTPUT_AND_RETURN ("Fehler beim Speichern des Videos");
			file->write (reply->readAll ());
			file->close ();

			outputMovie (file);
		} break;
	}
}

void WetterOnlineAnimationPlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	if (code==QNetworkReply::ContentNotFoundError)
	{
		switch ((State)state)
		{
			case stateNavigationPage:
				outputText (utf8 ("Fehler: Navigationsseite nicht gefunden (404)"));
				break;
			case stateRadarPage:
				outputText (utf8 ("Fehler: Radarseite nicht gefunden (404)"));
				break;
			case stateRadarImage:
				outputText (utf8 ("Fehler: Radarfilm nicht gefunden (404)"));
				break;
		}
	}
	else
	{
		outputText (reply->errorString ());
	}
}
