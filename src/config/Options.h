#ifndef _Options_h
#define _Options_h

/*
 * Options
 * Martin Herrmann
 * 2004-12-15
 */

// Adding an option: see doc/internal/Options

#include <QString>

#include <qdatetime.h>
#include <qfile.h>

#include "src/plugins/DataFormatPlugin.h"

enum dump_type_t { dt_invalid, dt_kein, dt_clublist, dt_liste, dt_flugbuch, dt_bordbuch };
enum dump_format_t { df_invalid, df_kein, df_latex, df_csv };
enum dump_columns_t { dc_invalid, dc_kein, dc_standard, dc_albgau };

class Database;
class ShellPlugin;

class Options
{
	public:
		Options ();
		bool parse_arguments (int argc, char *argv[]);
		bool read_config_file (QString filename, Database *db, QList<ShellPlugin> *plugins);
		bool read_config_files (Database *db, QList<ShellPlugin> *plugins, int argc, char *argv[]);
		static void display_options (QString prefix);
		void do_display ();
		bool need_display ();
		bool address_is_local (const QString &address) const;
		QString find_plugin_file (const QString &filename, QString *dir=NULL, QString *basename=NULL) const;

		// Configuration
		QString config_file;
		bool display_help;
		bool show_version;
		bool show_short_version;

		// Connection
		QString server;
		QString server_display_name;
		int port;
		QString database;
		QString username;
		QString password;
		QString root_name;
		QString root_password;
		QString sk_admin_name;
		QString sk_admin_password;

		// Local Options
		bool demosystem;
		QString title;
		QString diag_cmd;
		QString ort;
		QString style;

		// Networking Options
		QStringList local_hosts;

		// Debugging
		bool debug;
		bool display_queries;
		bool colorful;

		// Database
		bool record_towpilot;
		bool csv_quote;

		// Listing Options
		QStringList club_list;
		dump_type_t dump_type;
		dump_format_t dump_format;
		dump_columns_t dump_columns;

		// Date Options
		// TODO: replace QDate by something non-qt
		QDate date;
		QDate start_date;
		QDate end_date;

		// Plugin Entity
		QStringList plugin_paths;
		QList<DataFormatPlugin> plugins_data_format;

		// Weather plugin
		QString weather_plugin;
		int weather_height;
		int weather_interval;
		QString weather_dialog_plugin;
		QString weather_dialog_title;
		int weather_dialog_interval;

		// Non-Options
		QStringList non_options;
		QList<QDate> dates;

		// Parsing
		bool parse_type (const QString &type_string);
		bool parse_format (const QString &format_string);
		bool parse_columns (const QString &columns_string);
		bool parse_date (const QString &date_string, QDate &d, bool required);

		// Date manglement
		bool check_date () const;
		bool use_date_range () const;
		QDate eff_date () const;
		QDate eff_start_date () const;
		QDate eff_end_date () const;

		static bool silent;
};

extern Options opts;

#endif

