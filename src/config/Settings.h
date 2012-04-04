/*
 * Settings.h
 *
 *  Created on: 15.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QObject>
#include <QString>
#include <QStringList>

#include "src/db/DatabaseInfo.h"
#include "src/i18n/LanguageConfiguration.h"

class InfoPlugin;

/**
 * The new configuration class, to replace Options
 */
class Settings: public QObject
{
	Q_OBJECT

	public:
		virtual ~Settings ();
		static Settings &instance ();

		QStringList readArgs (const QStringList &args);
		void save ();

//		bool anyPluginsEnabled ();
//		void disableAllPlugins ();

	signals:
		void changed ();

	public:
		// *** Database
		// Connection
		DatabaseInfo databaseInfo;

		// *** Settings
		// UI
		LanguageConfiguration languageConfiguration;
		// Data
		QString location;
		bool recordTowpilot;
		bool checkMedicals;
		// Permissions
		bool protectSettings;
		bool protectLaunchMethods;
		bool protectMergePeople;
		bool protectFlightDatabase;
		bool protectViewMedicals;
		bool protectChangeMedicals;
		// Diagnostics
		bool enableDebug;
		QString diagCommand;
		bool coloredLabels;
		bool displayQueries;
		bool noFullScreen;
		bool enableShutdown;

		// *** Plugins - Info
		QList<InfoPlugin *> readInfoPlugins ();
		void writeInfoPlugins (const QList<InfoPlugin *> &plugins);

		// *** Plugins - Weather
		// Weather plugin
		QString weatherPluginId;
		QString weatherPluginCommand;
		bool weatherPluginEnabled;
		int weatherPluginHeight;
		int weatherPluginInterval; // in seconds
		// Weather dialog
		QString weatherWindowPluginId;
		QString weatherWindowCommand;
		bool weatherWindowEnabled;
		int weatherWindowInterval; // in seconds
		QString weatherWindowTitle;

		// *** Plugins - Paths
		QStringList pluginPaths;

		// *** Other
		QString programPath;

	protected:
		void readSettings ();
		void writeSettings ();

	private:
		static Settings *theInstance;
		Settings ();

};

#endif
