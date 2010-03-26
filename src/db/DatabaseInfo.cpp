#include "DatabaseInfo.h"

#include <QSettings>

#include <iostream>

DatabaseInfo::DatabaseInfo ():
	defaultPort (true), port (0)
{
}

DatabaseInfo::~DatabaseInfo ()
{
}

DatabaseInfo::DatabaseInfo (QSettings &settings):
	defaultPort (true), port (0)
{
	load (settings);
}


QString DatabaseInfo::toString () const
{
	return QString ("%1@%2:%3").arg (username, server, database);
}

DatabaseInfo::operator QString () const
{
	return toString ();
}

QString DatabaseInfo::serverText () const
{
	if (defaultPort)
		return server;
	else
		return QString ("%1:%2").arg (server).arg (port);
}

void DatabaseInfo::load (QSettings &settings)
{
	server     =settings.value ("server"     , "localhost"  ).toString ();
	defaultPort=settings.value ("defaultPort", true         ).toBool   ();
	port       =settings.value ("port"       , 3306         ).toInt    ();
	username   =settings.value ("username"   , "startkladde").toString ();
	password   =settings.value ("password"   , "moobert"    ).toString ();
	database   =settings.value ("database"   , "startkladde").toString ();
}

void DatabaseInfo::save (QSettings &settings)
{
	settings.setValue ("server"     , server     );
	settings.setValue ("defaultPort", defaultPort);
	settings.setValue ("port"       , port       );
	settings.setValue ("username"   , username   );
	settings.setValue ("password"   , password   );
	settings.setValue ("database"   , database   );
}
