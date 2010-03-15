/*
 * Settings.h
 *
 *  Created on: 15.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QObject>

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


//		// Local Options
//		bool demosystem;
//		QString title;
//		QString diag_cmd;
//		QString ort;
//		QString style;
//
//		// Debugging
//		bool debug;
//		bool display_queries;
//		bool colorful;
//
//		// Database
//		bool record_towpilot;
//		bool protect_launch_methods;
//
//		// Plugins
//		QStringList plugin_paths;
//		QList<ShellPlugin *> shellPlugins;
//
//		// Weather plugin
//		QString weather_plugin;
//		int weather_height;
//		int weather_interval;
//		QString weather_dialog_plugin;
//		QString weather_dialog_title;
//		int weather_dialog_interval;
//
//		// Non-Options
//		QStringList non_options;
//
//		static bool silent;
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
