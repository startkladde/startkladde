#include "Options.h"

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


//void Options::display_options (QString prefix)
//{
//	std::cout << prefix << "--config_file name, -f name: read config file name instead of the default" << std::endl;
//	std::cout << prefix << "--help, -h: display help" << std::endl;
//	std::cout << prefix << "--version: display version" << std::endl;
//	std::cout << prefix << "--server name, -s name: use database server name (IP/hostname)" << std::endl;
//	std::cout << prefix << "--server_display_name name: use name for display (e. g. when using a tunnel/proxy)" << std::endl;
//	std::cout << prefix << "--port num, -p num: connect to port num on the server" << std::endl;
//	std::cout << prefix << "--database name, -d name: use database name" << std::endl;
//	std::cout << prefix << "--username name, -u name: log in as user name" << std::endl;
//	std::cout << prefix << "--password pw, -p pw: log in with password pw" << std::endl;
//	std::cout << prefix << "--demosystem: show the demosystem menu" << std::endl;
//	std::cout << prefix << "--title text: show text in the window title (where applicable)" << std::endl;
//	std::cout << prefix << "--diag_cmd cmd: set the command for network diagnostics" << std::endl;
//	std::cout << prefix << "--ort o: the default value for departure/destination locations" << std::endl;
//	std::cout << prefix << "--style s: use QT style s, if available" << std::endl;
//	std::cout << prefix << "--debug: display" << std::endl;
//	std::cout << prefix << "--display_queries, -q: display all queries as they are executed" << std::endl;
//	std::cout << prefix << "--colorful: colorful display" << std::endl;
//}

//bool Options::parse_arguments (int argc, char *argv[])
//{
//	int c=0;
//	opterr=1;
//	optind=0;	// We might need to parse the Options multiple times
//
//	do
//	{
//		static struct option long_options[]=
//		{
//			// F for config_only?
//			// qQ for quiet
//			// vV for verbose
//
//			// Configuration
//			{ "config_file",     required_argument, NULL, 'f' },
//			{ "help",            no_argument,       NULL, 'h' },
//			{ "version",         no_argument,       NULL, opt_version },
//
//			// Connection
//			{ "server",              required_argument, NULL, 's' },
//			{ "server_display_name", required_argument, NULL, opt_server_display_name },
//			{ "port",                required_argument, NULL, 'o' },
//			{ "database",            required_argument, NULL, 'd' },
//			{ "username",            required_argument, NULL, 'u' },
//			{ "user_name",           required_argument, NULL, 'u' },
//			{ "password",            required_argument, NULL, 'p' },
//			{ "user_password",       required_argument, NULL, 'p' },
//
//			// Local Options
//			{ "demosystem",      no_argument,       NULL, opt_demosystem },
//			{ "title",           required_argument, NULL, opt_title },
//			{ "diag_cmd",        required_argument, NULL, opt_diag_cmd },
//			{ "ort",             required_argument, NULL, opt_ort },
//			{ "style",           required_argument, NULL, opt_style },
//
//			// Debugging
//			{ "debug",           no_argument,       NULL, opt_debug },
//			{ "display_queries", no_argument,       NULL, 'q' },
//			{ "colorful",        no_argument,       NULL, opt_colorful },
//
//			{ 0, 0, 0, 0 }
//		};
//		static const char *short_options="qf:hs:o:d:u:p:C:y:m:c:D:S:E:";
//
//		int option_index=0;
//		c=getopt_long (argc, argv, short_options, long_options, &option_index);
//
//		switch (c)
//		{
//			// Configuration
//			case 'f': config_file=optarg; break;
//			case 'h': display_help=true; break;
//			case opt_version: show_version=true; break;
//
//			// Connection
//			case 's': databaseInfo.server=optarg; server_display_name=optarg; break;
//			case 'o': databaseInfo.port=atoi (optarg); break;
//			case 'd': databaseInfo.database=optarg; break;
//			case 'u': databaseInfo.username=optarg; break;
//			case 'p': databaseInfo.password=optarg; break;
//			case opt_server_display_name: server_display_name=optarg; break;
//
//			// Local Options
//			case opt_demosystem: demosystem=true; break;
//			case opt_title: title=optarg; break;
//			case opt_diag_cmd: diag_cmd=optarg; break;
//			case opt_ort: ort=optarg; break;
//			case opt_style: style=optarg; break;
//
//			// Debugging
//			case opt_debug: debug=true; break;
//			case 'q': display_queries=true; break;
//			case opt_colorful: colorful=true; break;
//
//			// Errors
//			case '?': if (optopt=='?') display_help=true; else return false; break;
//			case ':': return false; break;
//			case -1: break;
//			default: std::cerr << "Error: unhandled option " << c << std::endl; return false; break;
//		}
//	} while (c>0);
//
//	non_options.clear ();
//
//	while (optind<argc)
//	{
//		non_options.push_back (argv[optind]);
//		optind++;
//	}
//
//	return true;
//}

QString Options::effectiveConfigFileName ()
{
	QString homeConfigFileName=get_environment ("HOME")+"/"+default_home_config_filename;
	if (QFile::exists (homeConfigFileName)) return homeConfigFileName;

	if (QFile::exists (default_local_config_fielname))
		return default_local_config_fielname;

	return "";
}

QList<LaunchMethod> Options::readConfiguredLaunchMethods ()
{
	QList<LaunchMethod> launchMethods;

	QString filename=effectiveConfigFileName ();
	if (filename.isEmpty ()) return launchMethods;

	QFile configFile (filename);

	if (!configFile.open (IO_ReadOnly))
	{
		std::cout << "Configuration file " << filename << " could not be opened" << std::endl;
		return launchMethods;
	}

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

			// Options not saved locally
			if (key=="startart")
			{
				LaunchMethod sa=LaunchMethod::parseConfigLine (value);
				if (idInvalid (sa.getId ()))
					std::cerr << "Error: launch method with invalid ID " << sa.getId () << " specified." << std::endl;
				else
					launchMethods.append (sa);
			}

		}
	}

	configFile.close ();

	return launchMethods;
}
