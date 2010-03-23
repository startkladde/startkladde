/*
 * TODO:
 *   - demosystem
 *   - allow setting some settings by both the config file and the command line
 */



#include "Settings.h"

#include <iostream>

#include <QSettings>

#include "src/util/qString.h"

Settings *Settings::theInstance=NULL;

/*
 * Notes:
 *   - don't store a QSettings instance in the class. Constructing and
 *     destructing QSettings instances is "very fast" (according to the
 *     documentation) and QSettings is only reentrant, not thread safe.
 */



Settings::Settings ():
	enableDebug (false), coloredLabels (false), displayQueries (false)
{
	readSettings ();
}

Settings::~Settings ()
{
}

Settings &Settings::instance ()
{
	if (!theInstance) theInstance=new Settings ();
	return *theInstance;
}

QStringList Settings::readArgs (const QStringList &args)
{
	QStringList unprocessed=args;
	unprocessed.removeFirst (); // remove argv[0]

	while (!unprocessed.isEmpty ())
	{
		QString arg=unprocessed.first ();

		if (arg.startsWith ("-"))
		{
			unprocessed.removeFirst ();

			if (arg=="-q")
				displayQueries=true;
			else if (arg=="--colored-labels")
				coloredLabels=true;
			else if (arg=="--no-full-screen")
				noFullScreen=true;
			else
				std::cout << "Unrecognized option " << arg << std::endl;
		}
		else
		{
			return unprocessed;
		}
	}

	return unprocessed;
}

void Settings::save ()
{
	writeSettings ();

	// Read back the changes so we can be sure we have the same state as when
	// reading the setups on startup.
	readSettings ();

	emit changed ();
}

void Settings::readSettings ()
{
	QSettings s;
	s.beginGroup ("settings");

	// *** Database
	s.beginGroup ("database");
	databaseInfo.load (s); // Connection
	s.endGroup ();

	// *** Settings
	// Data
	location      =s.value ("location"      , "Dingenskirchen").toString ();
	recordTowpilot=s.value ("recordTowpilot", true            ).toBool ();
	// Permissions
	protectSettings     =s.value ("protectSettings"     , false).toBool ();
	protectLaunchMethods=s.value ("protectLaunchMethods", false).toBool ();
	// Diagnostics
	enableDebug=s.value ("enableDebug", false                  ).toBool ();
	diagCommand=s.value ("diagCommand", "xterm -e ./netztest &").toString ();

	// *** Plugins - Info
	infoPlugins.clear ();
	if (s.contains ("infoPlugins/size"))
	{
		int n=s.beginReadArray ("infoPlugins");
		for (int i=0; i<n; ++i)
		{
			s.setArrayIndex (i);
			infoPlugins << ShellPluginInfo (s);
		}
		s.endArray ();
	}
	else
	{
		infoPlugins
			<< ShellPluginInfo ("Sunset:"         , "sunset_time sunsets"     , false, 0  , false)
			<< ShellPluginInfo ("Zeit bis sunset:", "sunset_countdown sunsets", true , 60 , false)
			<< ShellPluginInfo ("Wetter:"         , "metar EDDS"              , false, 600, false)
			<< ShellPluginInfo (""                , "metar EDDF"              , false, 600, false)
			<< ShellPluginInfo (""                , "metar EDFM"              , false, 600, false)
			;
	}


	// *** Plugins - Weather
	// Weather plugin
	weatherPluginCommand =s.value ("weatherPluginCommand" , "regenradar_wetteronline.de").toString ();
	weatherPluginHeight  =s.value ("weatherPluginHeight"  , 200).toInt ();
	weatherPluginInterval=s.value ("weatherPluginInterval", 600).toInt ();
	// Weather dialog
	weatherWindowCommand =s.value ("weatherWindowCommand" , "regenradar_wetteronline.de_ani").toString ();
	weatherWindowInterval=s.value ("weatherWindowInterval", 300).toInt ();
	weatherWindowTitle   =s.value ("weatherWindowTitle"   , "Regenradar (4 Stunden)").toInt ();

	// *** Plugins - Paths
	pluginPaths.clear ();
	if (s.contains ("pluginPaths/size"))
	{
		int n=s.beginReadArray ("pluginPaths");
		for (int i=0; i<n; ++i)
		{
			s.setArrayIndex (i);
			pluginPaths << s.value ("path").toString ();
		}
		s.endArray ();
	}
	else
	{
		pluginPaths
			<< "./.startkladde/plugins"
			<< "./plugins"
			<< "./plugins/shell_info"
			<< "./plugins/shell_info/sunset"
			<< "./plugins/weather"
			<< "/var/lib/startkladde/plugins"
			;
	}
}

void Settings::writeSettings ()
{
	QSettings s;
	s.beginGroup ("settings");

	// *** Database
	s.beginGroup ("database");
	databaseInfo.save (s); // Connection
	s.endGroup ();

	// *** Settings
	// Data
	s.setValue ("location"      , location      );
	s.setValue ("recordTowpilot", recordTowpilot);
	// Permissions
	s.setValue ("protectSettings"     , protectSettings     );
	s.setValue ("protectLaunchMethods", protectLaunchMethods);
	// Diagnostics
	s.setValue ("enableDebug", enableDebug);
	s.setValue ("diagCommand", diagCommand);

	// *** Plugins - Info
	s.beginWriteArray ("infoPlugins");
	for (int i=0; i<infoPlugins.size (); ++i)
	{
		s.setArrayIndex (i);
		infoPlugins.at (i).save (s);
	}
	s.endArray ();


	// *** Plugins - Weather
	// Weather plugin
	s.setValue ("weatherPluginCommand" , weatherPluginCommand);
	s.setValue ("weatherPluginHeight"  , weatherPluginHeight);
	s.setValue ("weatherPluginInterval", weatherPluginInterval);
	// Weather dialog
	s.setValue ("weatherWindowCommand" , weatherWindowCommand);
	s.setValue ("weatherWindowInterval", weatherWindowInterval);
	s.setValue ("weatherWindowTitle"   , weatherWindowTitle);

	// *** Plugins - Paths
	s.beginWriteArray ("pluginPaths");
	for (int i=0; i<pluginPaths.size (); ++i)
	{
		s.setArrayIndex (i);
		s.setValue ("path", pluginPaths.at (i));
	}
	s.endArray ();

	s.sync ();
}
