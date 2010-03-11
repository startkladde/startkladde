#ifndef OPTIONS_H_
#define OPTIONS_H_

// Adding an option: see doc/internal/Options

#include <QString>
#include <QStringList>
#include <QList>
#include <qdatetime.h>
#include <qfile.h>

#include "src/db/DatabaseInfo.h"

class ShellPlugin;
class LaunchMethod;

class Options
{
	public:
		Options ();
		bool parse_arguments (int argc, char *argv[]);
		bool read_config_file (QString filename);
		bool read_config_files (int argc, char *argv[]);
		static void display_options (QString prefix);
		void do_display ();
		bool need_display ();
		QString find_plugin_file (const QString &filename, QString *dir=NULL, QString *basename=NULL) const;

		// Configuration
		QString config_file;
		bool display_help;
		bool show_version;

		// Connection
		DatabaseInfo databaseInfo;
		QString server_display_name;

		// Local Options
		bool demosystem;
		QString title;
		QString diag_cmd;
		QString ort;
		QString style;

		// Debugging
		bool debug;
		bool display_queries;
		bool colorful;

		// Database
		bool record_towpilot;
		bool protect_launch_methods;

		// Plugins
		QStringList plugin_paths;
		QList<ShellPlugin *> shellPlugins;

		// Weather plugin
		QString weather_plugin;
		int weather_height;
		int weather_interval;
		QString weather_dialog_plugin;
		QString weather_dialog_title;
		int weather_dialog_interval;

		// Non-Options
		QStringList non_options;

		static bool silent;

		QList<LaunchMethod> configuredLaunchMethods;
};

extern Options opts;

#endif
