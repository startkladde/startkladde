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

/*
 * int size=settings.beginReadArray ("logins"); settings.setArrayIndex (i); settings.value (...)
 * beginWriteArray ("logins"), setArrayIndex, setValue
 * endArray ()
 *
 *
 */
Settings::Settings ()
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

void Settings::save ()
{
	writeSettings ();
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

	// *** Plugins - Weather
	// Weather plugin
	weatherPluginCommand =s.value ("weatherPluginCommand" , "plugins/weather/regenradar_wetter.com").toString ();
	weatherPluginHeight  =s.value ("weatherPluginHeight"  , 200).toInt ();
	weatherPluginInterval=s.value ("weatherPluginInterval", 600).toInt ();
	// Weather dialog
	weatherWindowCommand =s.value ("weatherWindowCommand" , "plugins/weather/regenradar_wetteronline.de_ani").toString ();
	weatherWindowInterval=s.value ("weatherWindowInterval", 300).toInt ();
	weatherWindowTitle   =s.value ("weatherWindowTitle"   , "Regenradar (4 Stunden)").toInt ();

	// *** Plugins - Paths
	pluginPaths.clear ();
	if (s.contains ("pluginPaths"))
	{
		int n=s.beginReadArray ("pluginPaths");
		for (int i=0; i<n; ++i)
			pluginPaths << s.value ("path").toString ();
		s.endArray ();
	}
	else
	{
		pluginPaths
			<< "./.startkladde/plugins"
			<< "./plugins"
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
