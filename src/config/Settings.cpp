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
 * QSettings settings;
 * settings.setValue ("foo/bar", variant) (overwrites)
 * settings.value ("foo/bar", default).toInt
 * beginGroup (), endGroup ()
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
	location      =s.value ("location"      , "Dingenskirchen").toString ();
	recordTowpilot=s.value ("recordTowpilot", true            ).toBool ();
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
	s.setValue ("location", location);
	s.setValue ("recordTowpilot", recordTowpilot);

	s.sync ();
}
