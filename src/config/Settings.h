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
		// Flarm
		bool flarmEnabled;
		bool flarmAutostart;
		bool flarmEditor;
		bool flarmOverview; // FIXME rename flarmPlaneList
		bool flarmRadar; // FIXME rename flarmMap?
		QString flarmMapKmlFileName;
		// FlarmNet
		bool flarmNetEnabled;
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

		// Override settings
		// If the database name is overridden (command line paramter), it won't
		// be written to the settings store. It is still possible to change it
		// in the GUI, though.
		bool overrideDatabaseName;
		QString overrideDatabaseNameValue;
		bool overrideDatabasePort;
		int overrideDatabasePortValue;

	protected:
		void readSettings ();
		void writeSettings ();

	private:
		static Settings *theInstance;
		Settings ();

};

#endif
