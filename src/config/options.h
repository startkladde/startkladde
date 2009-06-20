#ifndef options_h
#define options_h

/*
 * options
 * Martin Herrmann
 * 2004-12-15
 */

// Adding an option: see doc/internal/options

// Qt4
#include <qfile.h>

#include <string>
#include <qdatetime.h>
#include <list>
#include "src/plugins/plugin_data_format.h"

using namespace std;

enum dump_type_t { dt_invalid, dt_kein, dt_clublist, dt_liste, dt_flugbuch, dt_bordbuch };
enum dump_format_t { df_invalid, df_kein, df_latex, df_csv };
enum dump_columns_t { dc_invalid, dc_kein, dc_standard, dc_albgau };

class sk_db;
class sk_plugin;

class options
{
	public:
		options ();
		bool parse_arguments (int argc, char *argv[]);
		bool read_config_file (string filename, sk_db *db, list<sk_plugin> *plugins);
		bool read_config_files (sk_db *db, list<sk_plugin> *plugins, int argc, char *argv[]);
		static void display_options (string prefix);
		void do_display ();
		bool need_display ();
		bool address_is_local (const string &address) const;
		string find_plugin_file (const string &filename, string *dir=NULL, string *basename=NULL) const;

		// Configuration
		string config_file;
		bool display_help;
		bool show_version;
		bool show_short_version;

		// Connection
		string server;
		string server_display_name;
		int port;
		string database;
		string username;
		string password;
		string root_name;
		string root_password;
		string sk_admin_name;
		string sk_admin_password;

		// Local options
		bool demosystem;
		string title;
		string diag_cmd;
		string ort;
		string style;

		// Networking options
		list<string> local_hosts;

		// Debugging
		bool debug;
		bool display_queries;
		bool colorful;

		// Database
		bool record_towpilot;
		bool csv_quote;

		// Listing options
		list<string> club_list;
		dump_type_t dump_type;
		dump_format_t dump_format;
		dump_columns_t dump_columns;

		// Date options
		// TODO: replace QDate by something non-qt
		QDate date;
		QDate start_date;
		QDate end_date;

		// Plugin stuff
		list<string> plugin_paths;
		list<plugin_data_format> plugins_data_format;

		// Weather plugin
		string weather_plugin;
		int weather_height;
		int weather_interval;
		string weather_dialog_plugin;
		string weather_dialog_title;
		int weather_dialog_interval;

		// Non-options
		list<string> non_options;
		list<QDate> dates;

		// Parsing
		bool parse_type (const string &type_string);
		bool parse_format (const string &format_string);
		bool parse_columns (const string &columns_string);
		bool parse_date (const string &date_string, QDate &d, bool required);

		// Date manglement
		bool check_date () const;
		bool use_date_range () const;
		QDate eff_date () const;
		QDate eff_start_date () const;
		QDate eff_end_date () const;

		static bool silent;
};

extern options opts;

#endif

