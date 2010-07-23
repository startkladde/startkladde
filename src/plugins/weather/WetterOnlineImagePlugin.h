#ifndef WETTERONLINEIMAGEPLUGIN_H_
#define WETTERONLINEIMAGEPLUGIN_H_

#include "src/plugin/weather/WeatherPlugin.h"

#include <QNetworkReply> // Required for QNetworkReply::NetworkError

class Downloader;

class WetterOnlineImagePlugin: public WeatherPlugin
{
		SK_PLUGIN
		Q_OBJECT

	public:
		WetterOnlineImagePlugin ();
		virtual ~WetterOnlineImagePlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual void readSettings (const QSettings &settings) { (void)settings; }
		virtual void writeSettings (QSettings &settings) { (void)settings; }
		virtual PluginSettingsPane *createSettingsPane (QWidget *parent=NULL) { (void)parent; return NULL; }
		virtual QString configText () const { return QString (); }

	private:
		int state;
		Downloader *downloader;

	public slots:
		void refresh ();

		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);
};

#endif
