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

/**
 * The new configuration class, to replace Options
 */
class Settings: public QObject
{
	Q_OBJECT

	public:
		virtual ~Settings ();
		static Settings &instance ();

		void save ();

	signals:
		void changed ();

	public:
		// *** Database
		// Connection
		DatabaseInfo databaseInfo;

		// *** Settings
		// Data
		QString location;
		bool recordTowpilot;
		// Permissions
		bool protectSettings;
		bool protectLaunchMethods;
		// Diagnostics
		bool enableDebug;
		QString diagCommand;

		// *** Plugins - Info

		// *** Plugins - Weather
		// Weather plugin
		QString weatherPluginCommand;
		int weatherPluginHeight;
		int weatherPluginInterval;
		// Weather dialog
		QString weatherWindowCommand;
		int weatherWindowInterval;
		QString weatherWindowTitle;

		// *** Plugins - Paths
		QStringList pluginPaths;


//		bool demosystem;
//		bool display_queries;
//		bool colorful;
//		style <name> (-)
//		source <filename> (multi)
//
//		// Plugins
//		QList<ShellPlugin *> shellPlugins;
//
//		QList<LaunchMethod> configuredLaunchMethods;


	protected:
		void readSettings ();
		void writeSettings ();

	private:
		static Settings *theInstance;
		Settings ();

};

#endif
