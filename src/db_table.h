#ifndef db_table_h
#define db_table_h

/*
 * db_table
 * Martin Herrmann
 * 2004-12-13
 */

#include "db_column.h"
#include <list>
#include <iostream>

using namespace std;

class db_table
{
	public:
		db_table ();
		db_table (string _name);
		db_table (string _name, string _like_table);

		string name;
		string like_table;
		list<db_column> columns;
		string primary_key;
		list<string> unique_keys;

		string mysql_create_query (bool force=false) const;
};

ostream &operator<< (ostream &s, const db_table &c);

#endif

