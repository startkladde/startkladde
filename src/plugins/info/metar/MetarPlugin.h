#ifndef METARPLUGIN_H_
#define METARPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

#include <QNetworkReply> // Required for QNetworkReply::NetworkError
#include <QTimer>

/**
 * An info plugin which displays METAR messages downloaded from the internet
 *
 * Settings:
 *  - airport: the ICAO code of the airport to display the METAR of
 *  - refreshInterval: the interval for refreshing METAR messages in minutes
 */
class MetarPlugin: public InfoPlugin
{
		Q_OBJECT

	public:
		MetarPlugin (const QString &caption=QString (), bool enabled=true, const QString &airport="EDDF", int refreshInterval=15);
		virtual ~MetarPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual QString getId          () const;
		virtual QString getName        () const;
		virtual QString getDescription () const;

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		virtual QString configText () const;

		value_accessor (QString, Airport        , airport        );
		value_accessor (int    , RefreshInterval, refreshInterval);

	private:
		QString airport;
		int refreshInterval;
		QNetworkReply *reply;
		QTimer *timer;

//		QString extractMetar (const QString &reply);
		QString extractMetar (QIODevice &reply);

	private slots:
		void refresh ();
		void abortRequest ();

		void replyFinished ();
		void replyError (QNetworkReply::NetworkError code);
};

#endif
