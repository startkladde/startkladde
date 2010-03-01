#include "DatabaseInfo.h"

DatabaseInfo::DatabaseInfo ()
{
}

DatabaseInfo::~DatabaseInfo ()
{
}

QString DatabaseInfo::toString () const
{
	return QString ("%1@%2:%3").arg (username, server, database);
}

DatabaseInfo::operator QString () const
{
	return toString ();
}
