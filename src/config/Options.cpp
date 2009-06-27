#include "Options.h"

#include <cstdlib>
#include <fstream>

#include <getopt.h>

#include <QTextStream>

#include "src/version.h"
#include "src/db/Database.h"
#include "src/plugins/ShellPlugin.h"

const QString default_home_config_filename=".startkladde.conf";
const QString default_local_config_fielname="startkladde.conf";

// 4000 Connection
const int opt_server_display_name=4005;
const int opt_root_name=4001;
const int opt_root_pasword=4002;
const int opt_sk_admin_name=4003;
const int opt_sk_admin_password=4004;


// 3000 Configuration
const int opt_version=3001;
const int opt_short_version=3002;

// 1000 Local Options
const int opt_demosystem=1001;
const int opt_title=1002;
const int opt_diag_cmd=1003;
const int opt_ort=1004;
const int opt_style=1005;

// 2000 Debugging
const int opt_debug=2001;
const int opt_display_queries=2002;
const int opt_colorful=2003;

// 5000 Database
const int opt_record_towpilot=5001;

// Global variables
Options opts;

// Static members
bool Options::silent=false;

Options::Options ()
{
	// Set default values

	// Configuration
	config_file="";	// If this is empty, the defaults are tried.
	display_help=false;
	show_version=false;
	show_short_version=false;

	// Connection
	server="localhost";
	server_display_name=server;
	port=3306;
	database="startkladde";
	username="startkladde";
	password="";	// No default password as users are supposed to set their own.
	root_name="root";
	root_password="";	// No default password again.
	sk_admin_name="sk_admin";
	sk_admin_password="";	// No default password given.

	// Local Options
	demosystem=false;
	title="Startkladde";
	diag_cmd="";
	ort="Dingenskirchen";
	style="";	// If this is empty, the defaults are tried.
	colorful=false;

	// Listing Options
	dump_type=dt_liste;
	dump_format=df_csv;
	dump_columns=dc_standard;

	// Database
	record_towpilot=false;
	csv_quote = false;

	// Debugging
	debug=false;
	display_queries=false;

	// Weather plugin
	weather_plugin="";
	weather_height=200;
	weather_interval=600;
	weather_dialog_plugin="";
	weather_dialog_title="Wetter";
	weather_dialog_interval=600;
}

void Options::display_options (QString prefix)
{
	std::cout << prefix << "--config_file name, -f name: read config file name instead of the default" << std::endl;
	std::cout << prefix << "--help, -h: display help" << std::endl;
	std::cout << prefix << "--version: display version" << std::endl;
	std::cout << prefix << "--short_version: display short version" << std::endl;
	std::cout << prefix << "--server name, -s name: use database server name (IP/hostname)" << std::endl;
	std::cout << prefix << "--server_display_name name: use name for display (e. g. when using a tunnel/proxy)" << std::endl;
	std::cout << prefix << "--port num, -p num: connect to port num on the server" << std::endl;
	std::cout << prefix << "--database name, -d name: use database name" << std::endl;
	std::cout << prefix << "--username name, -u name: log in as user name" << std::endl;
	std::cout << prefix << "--password pw, -p pw: log in with password pw" << std::endl;
	std::cout << prefix << "--root_name name: use user name for root access (default: root)" << std::endl;
	std::cout << prefix << "--root_password pw: use pw as root password instead of prompting" << std::endl;
	std::cout << prefix << "--sk_admin_name name: use name for administration (default: sk_admin)" << std::endl;
	std::cout << prefix << "--sk_admin_password pw: use pw as sk_admin password instead of prompting" << std::endl;
	std::cout << prefix << "--demosystem: show the demosystem menu" << std::endl;
	std::cout << prefix << "--title text: show text in the window title (where applicable)" << std::endl;
	std::cout << prefix << "--diag_cmd cmd: set the command for network diagnostics" << std::endl;
	std::cout << prefix << "--ort o: the default value for Startort/Zielort" << std::endl;
	std::cout << prefix << "--style s: use QT style s, if available" << std::endl;
	std::cout << prefix << "--debug: display" << std::endl;
	std::cout << prefix << "--display_queries: display all queries as they are executed" << std::endl;
	std::cout << prefix << "--colorful: colorful display" << std::endl;
	std::cout << prefix << "--club name, -C name (multi): include club name" << std::endl;
	std::cout << prefix << "--type t, -t t: dump type t" << std::endl;
	std::cout << prefix << "--format f, -f f: dump format f" << std::endl;
	std::cout << prefix << "--columns c, -c c: dump columns c" << std::endl;
	std::cout << prefix << "--date d, -D d: use date D" << std::endl;
	std::cout << prefix << "--start_date s, -S s: start with date s" << std::endl;
	std::cout << prefix << "--end_date e, -E e: end with date e" << std::endl;
}

bool Options::parse_arguments (int argc, char *argv[])
{
	int c=0;
	opterr=1;
	optind=0;	// We might need to parse the Options multiple times

	do
	{
		static struct option long_options[]=
		{
			// F for config_only?
			// qQ for quiet
			// vV for verbose

			// Configuration
			{ "config_file",     required_argument, NULL, 'f' },
			{ "help",            no_argument,       NULL, 'h' },
			{ "version",         no_argument,       NULL, opt_version },
			{ "short_version",   no_argument,       NULL, opt_short_version },

			// Connection
			{ "server",              required_argument, NULL, 's' },
			{ "server_display_name", required_argument, NULL, opt_server_display_name },
			{ "port",                required_argument, NULL, 'o' },
			{ "database",            required_argument, NULL, 'd' },
			{ "username",            required_argument, NULL, 'u' },
			{ "user_name",           required_argument, NULL, 'u' },
			{ "password",            required_argument, NULL, 'p' },
			{ "user_password",       required_argument, NULL, 'p' },
			{ "root_name",           required_argument, NULL, opt_root_name },
			{ "root_password",       required_argument, NULL, opt_root_pasword },
			{ "sk_admin_name",       required_argument, NULL, opt_sk_admin_name },
			{ "sk_admin_password",   required_argument, NULL, opt_sk_admin_password },

			// Local Options
			{ "demosystem",      no_argument,       NULL, opt_demosystem },
			{ "title",           required_argument, NULL, opt_title },
			{ "diag_cmd",        required_argument, NULL, opt_diag_cmd },
			{ "ort",             required_argument, NULL, opt_ort },
			{ "style",           required_argument, NULL, opt_style },

			// Debugging
			{ "debug",           no_argument,       NULL, opt_debug },
			{ "display_queries", no_argument,       NULL, opt_display_queries },
			{ "colorful",        no_argument,       NULL, opt_colorful },

			// Listing Options
			{ "club",            required_argument, NULL, 'C' },
			{ "type",            required_argument, NULL, 'y' },
			{ "format",          required_argument, NULL, 'm' },
			{ "columns",         required_argument, NULL, 'c' },

			// Date Options
			{ "date",            required_argument, NULL, 'D' },
			{ "start_date",      required_argument, NULL, 'S' },
			{ "end_date",        required_argument, NULL, 'E' },

			{ 0, 0, 0, 0 }
		};
		static const char *short_options="f:hs:o:d:u:p:C:y:m:c:D:S:E:";

		int option_index=0;
		c=getopt_long (argc, argv, short_options, long_options, &option_index);

		switch (c)
		{
			// Configuration
			case 'f': config_file=optarg; break;
			case 'h': display_help=true; break;
			case opt_version: show_version=true; break;
			case opt_short_version: show_short_version=true; break;

			// Connection
			case 's': server=optarg; server_display_name=server; break;
			case opt_server_display_name: server_display_name=optarg; break;
			case 'o': port=atoi (optarg); break;
			case 'd': database=optarg; break;
			case 'u': username=optarg; break;
			case 'p': password=optarg; break;
			case opt_root_name: root_name=optarg; break;
			case opt_root_pasword: root_password=optarg; break;
			case opt_sk_admin_name: sk_admin_name=optarg; break;
			case opt_sk_admin_password: sk_admin_password=optarg; break;

			// Local Options
			case opt_demosystem: demosystem=true; break;
			case opt_title: title=optarg; break;
			case opt_diag_cmd: diag_cmd=optarg; break;
			case opt_ort: ort=optarg; break;
			case opt_style: style=optarg; break;

			// Debugging
			case opt_debug: debug=true; break;
			case opt_display_queries: display_queries=true; break;
			case opt_colorful: colorful=true; break;

			// Listing Options
			case 'C': club_list.push_back (optarg); break;
			case 'y': if (!parse_type (optarg)) return false; break;
			case 'm': if (!parse_format (optarg)) return false; break;
			case 'c': if (!parse_columns (optarg)) return false; break;

			// Date Options
			case 'D': if (!parse_date (optarg, date, true)) return false; break;
			case 'S': if (!parse_date (optarg, start_date, true)) return false; break;
			case 'E': if (!parse_date (optarg, end_date, true)) return false; break;

			// Errors
			case '?': if (optopt=='?') display_help=true; else return false; break;
			case ':': return false; break;
			case -1: break;
			default: std::cerr << "Error: unhandled option " << c << std::endl; return false; break;
		}
	} while (c>0);

	non_options.clear ();
	dates.clear ();

	while (optind<argc)
	{
		QDate d;

		if (parse_date (argv[optind], d, false))
		{
			dates.push_back (d);
		}

		non_options.push_back (argv[optind]);

		optind++;
	}

	return true;
}

bool Options::read_config_files (Database *db, QList<ShellPlugin> *plugins, int argc, char *argv[])
	// db is only passed for startrten (TODO change something)
	// Also reads command line arguments, if argc>0 (and a configuration file
	// was read)
	// Returns whether a configuration file was read.
{
	if (config_file.isEmpty ())
	{
		if (!read_config_file (get_environment ("HOME")+"/"+default_home_config_filename, db, plugins))
		{
			// The first default was not found, try the second.
			if (!read_config_file (default_local_config_fielname, db, plugins))
			{
				// The second default was't found either. We go on without a
				// config file, but return false.
				return false;
			}
		}
	}
	else
	{
		// Read the given configuration file.
		if (!read_config_file (config_file, db, plugins))
		{
			std::cerr << "Error: the specified configuration file " << config_file << " could not be read." << std::endl;
			return false;
		}
	}

	if (argc>0 && argv) parse_arguments (argc, argv);

	return true;
}

bool Options::read_config_file (QString filename, Database *db, QList<ShellPlugin> *plugins)
	// db is only passed for startarten (TODO change something)
	// Returns whether the file existed.
{
	QFile configFile (filename);

	if (!configFile.open (IO_ReadOnly))
	{
		if (!silent) std::cout << "Configuration file " << filename << " could not be opened" << std::endl;
		return false;
	}

	if (!silent) std::cout << "Reading configuration file " << filename << std::endl;

	QTextStream stream (&configFile);

	while (!stream.atEnd ())
	{
		std::string line=q2std (stream.readLine ().trimmed());

		if (line.length ()>0 && line[0]!='#')
		{
			QString key, value;
			// We use std::string here because it has find_first_of and find_first_not_of
			std::string::size_type pos=line.find_first_of (q2std (whitespace)); // TODO conversion is done for every line

			if (pos!=std::string::npos)
			{
				key=std2q (line.substr (0, pos));
				pos=line.find_first_not_of (q2std (whitespace), pos); // TODO conversion is done for every line
				value=std2q (line.substr (pos)).trimmed();
			}
			else
			{
				key=std2q (line);
			}

			// Connection
			if (key=="server") { server=value; server_display_name=server; }
			if (key=="server_display_name") { server_display_name=value; }
			if (key=="port") port=value.toInt ();
			if (key=="database") database=value;
			if (key=="username") username=value;
			if (key=="user_name") username=value;
			if (key=="password") password=value;
			if (key=="user_password") password=value;
			if (key=="sk_admin_name") sk_admin_name=value;
			if (key=="sk_admin_password") sk_admin_password=value;
			if (key=="root_name") root_name=value;
			if (key=="root_password") root_password=value;

			// Local Options
			if (key=="demosystem") demosystem=true;
			if (key=="title") title=value;
			if (key=="diag_cmd") diag_cmd=value;
			if (key=="ort") ort=value;
			if (key=="style") style=value;

			// Networking Options
			if (key=="local_hosts")
			{
				QStringList split=value.split (',');
				trim (split);
				local_hosts+=split;
			}

			// Database Options
			if (key=="record_towpilot") record_towpilot=true;

			// Plugin Entity
			if (key=="plugin_data_format")
			{
				plugins_data_format.push_back (DataFormatPlugin (value));
//				std::cerr << "Data format plugin: " << value << std::endl;
			}
			if (key=="plugin_path")
			{
				plugin_paths.push_back (value);
//				std::cerr << "Plugin path: " << value << std::endl;
			}

			if (key=="csv_quote")
			{
				csv_quote = true;
				std::cerr << "CSV quote: " << csv_quote << std::endl;
			}

			if (plugins && key=="shell_plugin")
			{
				ShellPlugin *p=new ShellPlugin (value);
				plugins->push_back (*p);
				if (!silent) std::cout << "Added shell plugin \"" << p->get_caption () << "\"" << std::endl;
			}

			// Weather plugins
			if (key=="weather_plugin") weather_plugin=value;
			if (key=="weather_height") weather_height=value.toInt();
			if (key=="weather_interval") weather_interval=value.toInt();
			if (key=="weather_dialog_plugin") weather_dialog_plugin=value;
			if (key=="weather_dialog_title") weather_dialog_title=value;
			if (key=="weather_dialog_interval") weather_dialog_interval=value.toInt();

			// Debugging
			if (key=="debug") debug=true;
			if (key=="colorful") colorful=true;

			// Actions
			if (key=="source")
			{
				bool ret=read_config_file (value, db, plugins);
				if (!ret) std::cerr << "Error: file \"" << value << "\" could not be read." << std::endl;
			}

			// Options not saved locally
			if (db && key=="startart")
			{
				LaunchType *sa=new LaunchType (value);
				if (id_invalid (sa->get_id ()))
				{
					std::cerr << "Error: startart with invalid ID " << sa->get_id () << " specified.";
					delete sa;
				}
				else
					db->add_startart_to_list (sa);
			}

		}
	}

	configFile.close ();

	return true;
}

void Options::do_display ()
{
	if (show_version)
		display_version ();
	else if (show_short_version)
		display_short_version ();
}

bool Options::need_display ()
{
	// No display_help because this is handled by the caller.
	return (show_version || show_short_version);
}

bool Options::parse_type (const QString &type_string)
{
	dump_type=dt_invalid;
	if (type_string=="list" || type_string=="l") dump_type=dt_liste;
	if (type_string=="liste" || type_string=="l") dump_type=dt_liste;
	if (type_string=="flugbuch" || type_string=="f") dump_type=dt_pilot_log;
	if (type_string=="bordbuch" || type_string=="b") dump_type=dt_plane_log;
	if (type_string=="clubs" || type_string=="c") dump_type=dt_clublist;
	if (type_string=="clublist" || type_string=="c") dump_type=dt_clublist;
	if (type_string=="club_list" || type_string=="c") dump_type=dt_clublist;
	if (dump_type==dt_invalid) std::cerr << "Invalid dump type specification \"" << type_string << "\"" << std::endl;
	return (dump_type!=dt_invalid);
}

bool Options::parse_format (const QString &format_string)
{
	dump_format=df_invalid;
	if (format_string=="latex" || format_string=="l") dump_format=df_latex;
	if (format_string=="csv" || format_string=="c") dump_format=df_csv;
	if (dump_format==df_invalid) std::cerr << "Invalid format type specification \"" << format_string << "\"" << std::endl;
	return (dump_format!=df_invalid);
}

bool Options::parse_columns (const QString &columns_string)
{
	dump_columns=dc_invalid;
	if (columns_string=="standard" || columns_string=="s") dump_columns=dc_standard;
	if (columns_string=="albgau" || columns_string=="a") dump_columns=dc_albgau;
	if (dump_columns==dc_invalid) std::cerr << "Invalid columns type specification \"" << columns_string << "\"" << std::endl;
	return (dump_columns!=dc_invalid);
}

bool Options::parse_date (const QString &date_string, QDate &d, bool required)
{
	d=QDate::fromString (date_string, Qt::ISODate);
	return d.isValid ();
}

bool Options::check_date () const
	// Checks dates. Outputs an error message on error.
	// Return value:
	//   - false: error
	//   - true: ok
{
	bool d=date.isValid ();        // A date was given
	bool s=start_date.isValid ();  // A start date was given
	bool e=end_date.isValid ();    // An end date was given
	bool exp=e||s||e;              // Any date was explicitly given
	int n=dates.size ();           // The number of non-option dates
	bool imp=(n>0);                // Any date was implicityly given

#define DATE_ERROR(condition, text) do { if ((condition)) { std::cerr << "Error: " << text << std::endl; return false; } } while (false);

	DATE_ERROR (exp && imp, "There were both explicit and non-option dates given.");
	DATE_ERROR (n>2, "There were more than 2 non-option dates given.");
	DATE_ERROR (d && (s || e), "There were both a date and a start/end date given.");
	DATE_ERROR (s && !e, "There was only a start date given.");
	DATE_ERROR (!s && e, "There was only an end date given.");
	return true;
}

bool Options::use_date_range () const
	/*
	 * Determines whether to use a date range or a single date.
	 * Conditions:
	 * Return value:
	 *   - true: date range
	 *   - false: single date
	 *   - if there is an error condition (check_date returns false), the
	 *     result is unspecified.
	 */
{
	if (dates.size ()==2) return true;
	if (start_date.isValid () && end_date.isValid ()) return true;
	return false;
}

QDate Options::eff_date () const
	/*
	 * Determines the effictive date to use for single date.
	 * Return value:
	 *   - the effective date
	 *   - if a date range is to be used (use_date_range returns true), the
	 *     result is undefined.
	 *   - if there is an error condition (check_date returns false), the
	 *     result is unspecified.
	 */
{
	if (!dates.empty ()) return dates.front ();
	if (date.isValid ()) return date;
	return QDate::currentDate ();
}

QDate Options::eff_start_date () const
	/*
	 * Determines the effective start date.
	 * Return value:
	 *   - the effective start date.
	 *   - if a single date is to be used (use_date_range returns false), this
	 *     is the same es eff_date.
	 *   - if there is an error condition (check_date returns false), the
	 *     result is unspecified.
	 */
{
	if (!use_date_range ()) return eff_date ();
	if (!dates.empty ()) return dates.front ();
	return start_date;
}

QDate Options::eff_end_date () const
	/*
	 * Determines the effective end date.
	 * Return value:
	 *   - the effective end date.
	 *   - if a single date is to be used (use_date_range returns false), this
	 *     is the same es eff_date.
	 *   - if there is an error condition (check_date returns false), the
	 *     result is unspecified.
	 */
{
	if (!use_date_range ()) return eff_date ();
	if (!dates.empty ()) return dates.back ();
	return end_date;
}

bool Options::address_is_local (const QString &address) const
	/*
	 * Checks whether an address is to be regarded local.
	 * Parameters:
	 *   - address: the address to be checked. No further whitespace removal is
	 *     done.
	 * Return value:
	 *   - true if address is local
	 *   - false else.
	 */
{
	QStringListIterator it (local_hosts);

	while (it.hasNext ())
	{
		QString l=it.next();
		if (l.at (l.length ()-1)=='*')
		{
			// The local address specifcation ends with a *, thus is an address
			// range. Check if the given address starts with the local address
			// specification (without the *).
			int relevant_length=l.length ()-1;
			if (l.left (relevant_length)==address.left (relevant_length)) return true;
		}
		else
		{
			// The local address specification is a single address. Check if it
			// matches exactly.
			if (l==address) return true;
		}
	}

	return false;
}

QString Options::find_plugin_file (const QString &filename, QString *dir, QString *basename) const
	// The file returned exists or else the name is empty.
	// *dir will be set to the dir if the result is not empty.
{
	// Empty file names are empty file names.
	if (filename.isEmpty ()) return "";
	if (filename.indexOf ('/')>=0)
	{
		// The name contains a '/' which means that it is an absolute or relative
		if (QFile::exists (filename))
		{
			// Find the last slash: it separates the path from the basename
			// Absolute: /foo/bar/baz
			// Relative: bar/baz
			//           0123456
			unsigned int last_slash_pos=filename.lastIndexOf ('/');
			// last_slash_pos cannot be npos becase there is a slash
			if (dir) *dir=filename.left (last_slash_pos);
			if (basename) *basename=filename.mid (last_slash_pos+4);
			return filename;
		}
		else
			return "";
	}
	else if (plugin_paths.empty ())
	{
		// Pugin path is empty
		if (QFile::exists (QString ("./")+filename))
		{
			if (dir) *dir=".";
			if (basename) *basename=filename;
			return "./"+filename;
		}
		else
			return "";
	}
	else
	{
		// So we have to search the plugin path.
		QStringListIterator it (plugin_paths);

		while (it.hasNext ())
		{
			QString path_entry=it.next ();
			if (QFile::exists (path_entry+"/"+filename))
			{
				if (dir) *dir=path_entry;
				if (basename) *basename=filename;
				return *path_entry+"/"+filename;
			}
		}
	}

	// Nothing found.
	return "";
}

