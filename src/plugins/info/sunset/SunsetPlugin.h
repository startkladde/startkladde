/*
 * SunsetPlugin.h
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#ifndef SunsetPlugin_H_
#define SunsetPlugin_H_

#include <QTime>

#include "src/plugin/info/InfoPlugin.h"

/**
 * A plugin which displays the sunset time for the current date, read from a
 * data file
 *
 * Settings:
 *  - filename: the name of the file to read the data from
 */
class SunsetPlugin: public InfoPlugin
{
	public:
		friend class SunsetPluginSettingsPane;

		SunsetPlugin (QString caption=QString (), bool enabled=true, const QString &filename="sunsets.txt");
		virtual ~SunsetPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual QString getId          () const;
		virtual QString getName        () const;
		virtual QString getDescription () const;

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, Filename, filename);

		virtual QString configText () const;

		virtual QString findSunset ();

	private:

		QString filename;
};

#endif
