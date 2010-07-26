/*
 * WetterOnlineImagePlugin.cpp
 *
 *  Created on: 23.07.2010
 *      Author: martin
 */

#include "WetterOnlineImagePlugin.h"

#include <QImage>
#include <QRegExp>

#include "src/plugin/factory/PluginFactory.h"
#include "src/net/Downloader.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/text.h"

REGISTER_PLUGIN (WeatherPlugin, WetterOnlineImagePlugin)
SK_PLUGIN_DEFINITION (WetterOnlineImagePlugin, "{a00e31ec-6d3d-4221-91bd-751a2756937f}", "Wetter Online (Bild)", "Zeigt ein Regenradarbild von wetteronline.de an")

enum State { stateIndexPage, stateImage };

const QString indexUrl ("http://www.wetteronline.de/daten/radarhtml/de/dwddg/radarf.htm");

WetterOnlineImagePlugin::WetterOnlineImagePlugin ():
	downloader (new Downloader (this))
{
	downloader->connectSignals (this);
}

WetterOnlineImagePlugin::~WetterOnlineImagePlugin ()
{
}

void WetterOnlineImagePlugin::refresh ()
{
	outputText (utf8 ("Radarbild herunterladen (1)..."));
	downloader->startDownload (stateIndexPage, indexUrl);
}

void WetterOnlineImagePlugin::abort ()
{
	downloader->abort ();
}

void WetterOnlineImagePlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
	switch ((State)state)
	{
		case stateIndexPage:
		{
			QString imagePath=findInIoDevice (*reply, QRegExp ("(daten\\/radar[^\"]*)\""), 1);
			if (isBlank (imagePath)) OUTPUT_AND_RETURN ("Fehler: keine Wettergrafik gefunden");
			QString imageUrl=QString ("http://www.wetteronline.de/%1").arg (imagePath);
			downloader->startDownload (stateImage, imageUrl);
			outputText (utf8 ("Radarbild herunterladen (2)..."));
		} break;
		case stateImage:
		{
			QByteArray data=reply->readAll ();
			QImage image=QImage::fromData (data);
			if (image.isNull ()) OUTPUT_AND_RETURN ("Fehler: ungültige Wettergrafik");
			outputImage (image);
		} break;
	}
}

void WetterOnlineImagePlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	if (code==QNetworkReply::ContentNotFoundError)
	{
		switch ((State)state)
		{
			case stateIndexPage:
				outputText (utf8 ("Fehler: Wetterseite nicht gefunden (404)"));
				break;
			case stateImage:
				outputText (utf8 ("Fehler: Wettergrafik nicht gefunden (404)"));
				break;
		}
	}
	else
	{
		outputText (reply->errorString ());
	}
}
