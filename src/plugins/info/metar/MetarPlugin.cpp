/*
 * MetarPlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "MetarPlugin.h"

//#include <QDebug>
#include <QSettings>
#include <QRegExp>
#include <QString>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "src/plugin/info/InfoPluginFactory.h"
#include "MetarPluginSettingsPane.h"
#include "src/util/qString.h"

REGISTER_INFO_PLUGIN (MetarPlugin)

MetarPlugin::MetarPlugin (const QString &caption, bool enabled, const QString &airport, int refreshInterval):
	InfoPlugin (caption, enabled),
	airport (airport), refreshInterval (refreshInterval),
	reply (NULL), timer (new QTimer (this))
{
	connect (timer, SIGNAL (timeout ()), this, SLOT (refresh ()));
}

MetarPlugin::~MetarPlugin ()
{
}

QString MetarPlugin::getId () const
{
	return "metar";
}

QString MetarPlugin::getName () const
{
	return "METAR";
}

QString MetarPlugin::getDescription () const
{
	return utf8 ("Zeigt METAR-Meldungen an (Internetverbindung erforderlich)");
}

PluginSettingsPane *MetarPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new MetarPluginSettingsPane (this, parent);
}

void MetarPlugin::infoPluginReadSettings (const QSettings &settings)
{
	airport         =settings.value ("airport"        , airport        ).toString ();
	refreshInterval =settings.value ("refreshInterval", refreshInterval).toInt    ();
}

void MetarPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("airport"        , airport        );
	settings.setValue ("refreshInterval", refreshInterval);
}

void MetarPlugin::start ()
{
	refresh ();

	if (timer->isActive ()) timer->stop ();
	timer->start (60*1000*refreshInterval);
}

void MetarPlugin::terminate ()
{
	abortRequest ();
	timer->stop ();
}

QString MetarPlugin::configText () const
{
	return utf8 ("%1 (%2 Minuten)").arg (airport).arg (refreshInterval);
}
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

void MetarPlugin::abortRequest ()
{
	if (reply)
	{
		reply->abort ();
		reply->deleteLater ();
		reply=NULL;
	}
}

void MetarPlugin::refresh ()
{
	abortRequest ();

	QString icao=airport.trimmed ().toUpper ();

	if (icao.isEmpty ())
		outputText (utf8 ("Kein Flughafen angegeben"));
	if (!airport.contains (QRegExp ("^[A-Z]{4,4}$")))
		outputText (utf8 ("%1 ist kein gültiger ICAO-Code").arg (airport));
	else
	{
		outputText (utf8 ("Rufe METAR für %1 ab...").arg (icao));

		QString url=QString ("http://weather.noaa.gov/mgetmetar.php?cccc=%1").arg (icao);

		// FIXME global manager
		QNetworkAccessManager *manager=new QNetworkAccessManager (this);

		QNetworkRequest request;
		request.setUrl (QUrl (url));
		//request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

		reply=manager->get (request);
		connect (reply, SIGNAL (finished ()), this, SLOT (replyFinished ()));
		connect(
			reply, SIGNAL (error (QNetworkReply::NetworkError)),
			this, SLOT(replyError (QNetworkReply::NetworkError)));
	}
}

QString MetarPlugin::extractMetar (const QString &reply)
{
	// The relevant section of the page:
	//
	//<font face="courier" size = "5">
	//EDDF 311920Z 26009KT 9999 FEW012 SCT019 BKN023 13/11 Q1015 NOSIG
	//
	//</font>
	// FIXME use IODevice.readLine?
	foreach (const QString &line, reply.split ("\n"))
		if (line.startsWith (airport.trimmed ().toUpper ()))
			return line;

	return QString ();
}

void MetarPlugin::replyFinished ()
{
	if (!reply) return; // May happen if the plugin is terminated while the request is still active
	if (sender ()!=reply) return;

	QString metar=extractMetar (reply->readAll ());

	if (metar.isEmpty ())
		outputText (utf8 ("Fehler: Keine Wettermeldung für %1 gefunden").arg (airport));
	else
		outputText (metar);

	reply->deleteLater ();
	reply=NULL;
}

void MetarPlugin::replyError (QNetworkReply::NetworkError code)
{
	if (!reply) return; // May happen if the plugin is terminated while the request is still active
	if (sender ()!=reply) return;

	if (code==QNetworkReply::ContentNotFoundError)
		outputText (utf8 ("Fehler: Wetterseite nicht gefunden (404)"));
	else
		outputText (reply->errorString ());

	reply->deleteLater ();
	reply=NULL;
}
