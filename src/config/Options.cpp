#include "Options.h"

//#include <cstdlib>
//#include <fstream>
//
#include <getopt.h> // TODO remove
#include <cstdlib> // for atoi, TODO remove

#include <iostream>

#include <QTextStream>

#include "src/text.h"
#include "src/version.h"
#include "src/plugins/ShellPlugin.h"
#include "src/model/LaunchMethod.h"
#include "src/util/qString.h"

const QString default_home_config_filename=".startkladde.conf";
const QString default_local_config_fielname="startkladde.conf";

// 4000 Connection
const int opt_server_display_name=4005;


// 3000 Configuration
const int opt_version=3001;

// 1000 Local Options
const int opt_demosystem=1001;
const int opt_title=1002;
const int opt_diag_cmd=1003;
const int opt_ort=1004;
const int opt_style=1005;

// 2000 Debugging
const int opt_debug=2001;
//const int opt_display_queries=2002;
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

	// Connection
	databaseInfo.server="localhost";
	databaseInfo.database="startkladde";
	databaseInfo.username="startkladde";
	databaseInfo.password="";	// No default password as users are supposed to set their own.
	databaseInfo.port=3306;

	server_display_name=databaseInfo.server;

	// Local Options
	demosystem=false;
	title="Startkladde";
	diag_cmd="";
	ort="Dingenskirchen";
	style="";	// If this is empty, the defaults are tried.
	colorful=false;

	// Database
	record_towpilot=false;
	protect_launch_methods=false;

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
	std::cout << prefix << "--server name, -s name: use database server name (IP/hostname)" << std::endl;
	std::cout << prefix << "--server_display_name name: use name for display (e. g. when using a tunnel/proxy)" << std::endl;
	std::cout << prefix << "--port num, -p num: connect to port num on the server" << std::endl;
	std::cout << prefix << "--database name, -d name: use database name" << std::endl;
	std::cout << prefix << "--username name, -u name: log in as user name" << std::endl;
	std::cout << prefix << "--password pw, -p pw: log in with password pw" << std::endl;
	std::cout << prefix << "--demosystem: show the demosystem menu" << std::endl;
	std::cout << prefix << "--title text: show text in the window title (where applicable)" << std::endl;
	std::cout << prefix << "--diag_cmd cmd: set the command for network diagnostics" << std::endl;
	std::cout << prefix << "--ort o: the default value for departure/destination locations" << std::endl;
	std::cout << prefix << "--style s: use QT style s, if available" << std::endl;
	std::cout << prefix << "--debug: display" << std::endl;
	std::cout << prefix << "--display_queries, -q: display all queries as they are executed" << std::endl;
	std::cout << prefix << "--colorful: colorful display" << std::endl;
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

			// Connection
			{ "server",              required_argument, NULL, 's' },
			{ "server_display_name", required_argument, NULL, opt_server_display_name },
			{ "port",                required_argument, NULL, 'o' },
			{ "database",            required_argument, NULL, 'd' },
			{ "username",            required_argument, NULL, 'u' },
			{ "user_name",           required_argument, NULL, 'u' },
			{ "password",            required_argument, NULL, 'p' },
			{ "user_password",       required_argument, NULL, 'p' },

			// Local Options
			{ "demosystem",      no_argument,       NULL, opt_demosystem },
			{ "title",           required_argument, NULL, opt_title },
			{ "diag_cmd",        required_argument, NULL, opt_diag_cmd },
			{ "ort",             required_argument, NULL, opt_ort },
			{ "style",           required_argument, NULL, opt_style },

			// Debugging
			{ "debug",           no_argument,       NULL, opt_debug },
			{ "display_queries", no_argument,       NULL, 'q' },
			{ "colorful",        no_argument,       NULL, opt_colorful },

			{ 0, 0, 0, 0 }
		};
		static const char *short_options="qf:hs:o:d:u:p:C:y:m:c:D:S:E:";

		int option_index=0;
		c=getopt_long (argc, argv, short_options, long_options, &option_index);

		switch (c)
		{
			// Configuration
			case 'f': config_file=optarg; break;
			case 'h': display_help=true; break;
			case opt_version: show_version=true; break;

			// Connection
			case 's': databaseInfo.server=optarg; server_display_name=optarg; break;
			case 'o': databaseInfo.port=atoi (optarg); break;
			case 'd': databaseInfo.database=optarg; break;
			case 'u': databaseInfo.username=optarg; break;
			case 'p': databaseInfo.password=optarg; break;
			case opt_server_display_name: server_display_name=optarg; break;

			// Local Options
			case opt_demosystem: demosystem=true; break;
			case opt_title: title=optarg; break;
			case opt_diag_cmd: diag_cmd=optarg; break;
			case opt_ort: ort=optarg; break;
			case opt_style: style=optarg; break;

			// Debugging
			case opt_debug: debug=true; break;
			case 'q': display_queries=true; break;
			case opt_colorful: colorful=true; break;

			// Errors
			case '?': if (optopt=='?') display_help=true; else return false; break;
			case ':': return false; break;
			case -1: break;
			default: std::cerr << "Error: unhandled option " << c << std::endl; return false; break;
		}
	} while (c>0);

	non_options.clear ();

	while (optind<argc)
	{
		non_options.push_back (argv[optind]);
		optind++;
	}

	return true;
}

bool Options::read_config_files (int argc, char *argv[])
	// Also reads command line arguments, if argc>0 (and a configuration file
	// was read)
	// Returns whether a configuration file was read.
{
	if (config_file.isEmpty ())
	{
		if (!read_config_file (get_environment ("HOME")+"/"+default_home_config_filename))
		{
			// The first default was not found, try the second.
			if (!read_config_file (default_local_config_fielname))
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
		if (!read_config_file (config_file))
		{
			std::cerr << "Error: the specified configuration file " << config_file << " could not be read." << std::endl;
			return false;
		}
	}

	if (argc>0 && argv) parse_arguments (argc, argv);

	return true;
}

bool Options::read_config_file (QString filename)
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
			if (key=="server"   ) { databaseInfo.server=value; server_display_name=value; }
			if (key=="port"     ) databaseInfo.port=value.toInt ();
			if (key=="database" ) databaseInfo.database=value;
			if (key=="username" ) databaseInfo.username=value;
			if (key=="user_name") databaseInfo.username=value;
			if (key=="password" ) databaseInfo.password=value;
			if (key=="server_display_name") { server_display_name=value; }
			if (key=="user_password") databaseInfo.password=value;

			// Local Options
			if (key=="demosystem") demosystem=true;
			if (key=="title") title=value;
			if (key=="diag_cmd") diag_cmd=value;
			if (key=="ort") ort=value;
			if (key=="style") style=value;

			// Database Options
			if (key=="record_towpilot") record_towpilot=true;
			if (key=="protect_launch_methods") protect_launch_methods=true;

			// Plugins
			if (key=="plugin_path")
			{
				plugin_paths.push_back (value);
//				std::cerr << "Plugin path: " << value << std::endl;
			}

			if (key=="shell_plugin")
			{
				ShellPlugin *p=new ShellPlugin (value);
				shellPlugins.push_back (p);
//				if (!silent) std::cout << "Added shell plugin \"" << p->get_caption () << "\"" << std::endl;
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
				bool ret=read_config_file (value);
				if (!ret) std::cerr << "Error: file \"" << value << "\" could not be read." << std::endl;
			}

			// Options not saved locally
			if (key=="startart")
			{
				LaunchMethod sa=LaunchMethod::parseConfigLine (value);
				if (idInvalid (sa.getId ()))
					std::cerr << "Error: launch method with invalid ID " << sa.getId () << " specified." << std::endl;
				else
					configuredLaunchMethods.append (sa);
			}

		}
	}

	configFile.close ();

	return true;
}

void Options::do_display ()
{
	if (show_version)
		std::cout << getVersion () << std::endl;
}

bool Options::need_display ()
{
	// No display_help because this is handled by the caller.
	return show_version;
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

