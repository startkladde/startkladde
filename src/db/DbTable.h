#ifndef _DbTable_h
#define _DbTable_h

/*
 * DbTable
 * Martin Herrmann
 * 2004-12-13
 */

#include <iostream>

#include "src/db/DbColumn.h"

class dbTable
{
	public:
		dbTable ();
		dbTable (QString _name);
		dbTable (QString _name, QString _like_table);

		QString name;
		QString like_table;
		QList<DbColumn> columns;
		QString primary_key;
		QStringList unique_keys;

		QString mysql_create_query (bool force=false) const;
};

std::ostream &operator<< (std::ostream &s, const dbTable &c);

#endif

