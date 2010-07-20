/*
 * TODO:
 *   - allow setting some settings by both the config file and the command line
 */



#include "Settings.h"

#include <iostream>

#include <QSettings>

#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/info/InfoPluginFactory.h"
#include "src/plugins/info/test/TestPlugin.h"
#include "src/plugins/info/metar/MetarPlugin.h"
#include "src/plugins/info/sunset/SunsetTimePlugin.h"
#include "src/plugins/info/sunset/SunsetCountdownPlugin.h"


Settings *Settings::theInstance=NULL;

/*
 * Notes:
 *   - don't store a QSettings instance in the class. Constructing and
 *     destroying QSettings instances is "very fast" (according to the
 *     documentation) and QSettings is only reentrant, not thread safe.
 */



Settings::Settings ():
	// Settings which are read from the configuration file do not have to be
	// initialized at this point because they are set to default values in
	// readSettings. All settings which are only set by command line have to be
	// initialized.
	enableDebug (false), coloredLabels (false), displayQueries (false),
	noFullScreen (false), enableShutdown (false)
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
			else if (arg=="--enable-shutdown")
				enableShutdown=true;
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

QList<InfoPlugin *> Settings::readInfoPlugins ()
{
	QList<InfoPlugin *> plugins;

	QSettings s;
	s.beginGroup ("settings");

	InfoPluginFactory &factory=InfoPluginFactory::getInstance ();

	// If no entry for infoPlugins exists, create a default set of plugins.
	// Note that if no plugins are used, there is still an entry with 0
	// elements.
	if (s.contains ("infoPlugins/size"))
	{
		int n=s.beginReadArray ("infoPlugins");
		for (int i=0; i<n; ++i)
		{
			s.setArrayIndex (i);

			QString id=s.value ("id").toString ();
			InfoPlugin *plugin=factory.create (id);

			// TODO better handling if not found
			if (plugin)
			{
				s.beginGroup ("settings");
				plugin->readSettings (s);
				s.endGroup ();

				plugins << plugin;
			}
		}
		s.endArray ();
	}
	else
	{
//		plugins.append (new TestPlugin ("Foo:"));
//		plugins.append (new TestPlugin ("Bar:", true, "TestPlugin", true));

		plugins.append (new SunsetTimePlugin      ("Sunset:"         , true, "sunsets.txt"));
		plugins.append (new SunsetCountdownPlugin ("Zeit bis sunset:", true, "sunsets.txt"));

		plugins.append (new MetarPlugin ("Wetter:", true, "EDDF", 15));
		plugins.append (new MetarPlugin (""       , true, "EDDS", 15));
		plugins.append (new MetarPlugin (""       , true, "EDDM", 15));
	}

	return plugins;
}

void Settings::writeInfoPlugins (const QList<InfoPlugin *> &plugins)
{
	QSettings s;
	s.beginGroup ("settings");

	// *** Plugins - Info
	s.beginWriteArray ("infoPlugins");
	for (int i=0; i<plugins.size (); ++i)
	{
		s.setArrayIndex (i);

		InfoPlugin *plugin=plugins[i];
		s.setValue ("id", plugin->getId ().toString ());

		s.beginGroup ("settings");
		plugin->writeSettings (s);
		s.endGroup ();
	}
	s.endArray ();

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
	enableDebug=s.value ("enableDebug", false       ).toBool ();
	diagCommand=s.value ("diagCommand", "./script/netztest_xterm").toString (); // xterm -e ./netztest &

	// *** Plugins - Weather
	// Weather plugin
	weatherPluginCommand =s.value ("weatherPluginCommand" , "regenradar_wetteronline.de.rb").toString ();
	weatherPluginEnabled =s.value ("weatherPluginEnabled" , true).toBool ();
	weatherPluginHeight  =s.value ("weatherPluginHeight"  , 200).toInt ();
	weatherPluginInterval=s.value ("weatherPluginInterval", 600).toInt ();
	// Weather dialog
	weatherWindowCommand =s.value ("weatherWindowCommand" , "regenradar_wetteronline.de_animation.rb").toString ();
	weatherWindowEnabled =s.value ("weatherWindowEnabled" , true).toBool ();
	weatherWindowInterval=s.value ("weatherWindowInterval", 300).toInt ();
	weatherWindowTitle   =s.value ("weatherWindowTitle"   , "Regenradar (3 Stunden)").toString ();

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
			<< "./plugins/info"
			<< "./plugins/weather"
			<< "/usr/lib/startkladde/plugins"
			<< "/usr/lib/startkladde/plugins/info"
			<< "/usr/lib/startkladde/plugins/weather"
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


	// *** Plugins - Weather
	// Weather plugin
	s.setValue ("weatherPluginCommand" , weatherPluginCommand);
	s.setValue ("weatherPluginEnabled" , weatherPluginEnabled);
	s.setValue ("weatherPluginHeight"  , weatherPluginHeight);
	s.setValue ("weatherPluginInterval", weatherPluginInterval);
	// Weather dialog
	s.setValue ("weatherWindowCommand" , weatherWindowCommand);
	s.setValue ("weatherWindowEnabled" , weatherWindowEnabled);
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

//bool Settings::anyPluginsEnabled ()
//{
//	if (weatherPluginEnabled) return true;
//	if (weatherWindowEnabled) return true;
//
//	foreach (const ShellPluginInfo &plugin, infoPlugins)
//		if (plugin.enabled)
//			return true;
//
//	return false;
//}

//void Settings::disableAllPlugins ()
//{
//	weatherPluginEnabled=false;
//	weatherWindowEnabled=false;
//
//	for (int i=0; i<infoPlugins.size (); ++i)
//		infoPlugins[i].enabled=false;
//}
