#ifndef db_table_h
#define db_table_h

/*
 * db_table
 * Martin Herrmann
 * 2004-12-13
 */

#include <iostream>
#include <list>

#include "src/db/db_column.h"


class db_table
{
	public:
		db_table ();
		db_table (QString _name);
		db_table (QString _name, QString _like_table);

		QString name;
		QString like_table;
		std::list<db_column> columns;
		QString primary_key;
		QStringList unique_keys;

		QString mysql_create_query (bool force=false) const;
};

std::ostream &operator<< (std::ostream &s, const db_table &c);

#endif

