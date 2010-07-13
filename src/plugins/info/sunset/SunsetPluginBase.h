#ifndef SunsetPluginBase_H_
#define SunsetPluginBase_H_

#include <QTime>

#include "src/plugin/info/InfoPlugin.h"
#include "src/Longitude.h"

/**
 * A plugin which displays the sunset time for the current date, read from a
 * data file
 *
 * Settings:
 *  - filename: the name of the file to read the data from
 */
class SunsetPluginBase: public InfoPlugin
{
	public:
		friend class SunsetPluginSettingsPane;

		SunsetPluginBase (QString caption=QString (), bool enabled=true, const QString &filename="sunsets.txt");
		virtual ~SunsetPluginBase ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, Filename, filename);
		value_accessor (Longitude, Longitude, longitude);
		value_accessor (bool, LongitudeCorrection, longitudeCorrection);

		virtual QString configText () const;

		virtual void start ();
		virtual void terminate ();

		static Longitude findReferenceLongitude (const QString &filename, bool *ok=NULL);
		static QString findSource (const QString &filename);

	protected:
		virtual QString findSunset ();

		QString sunset;
		bool sunsetValid;

	private:
		QString filename;
		Longitude longitude;
		bool longitudeCorrection;
};

#endif
