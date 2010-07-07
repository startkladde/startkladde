#ifndef SunsetPluginBase_H_
#define SunsetPluginBase_H_

#include <QTime>

#include "src/plugin/info/InfoPlugin.h"

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

		virtual QString configText () const;

		virtual void start ();
		virtual void terminate ();

	protected:
		virtual QString findSunset ();

		QString sunset;
		bool sunsetValid;

	private:
		QString filename;
};

#endif
