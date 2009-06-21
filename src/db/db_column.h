#ifndef db_column_h
#define db_column_h

/*
 * db_column
 * Martin Herrmann
 * 2004-12-12
 */

#include <iostream>
#include <string>

#include <mysql.h>

#include "src/text.h"

using namespace std;

// Flags: NOT_NULL_FLAG, PRI_KEY_FLAG, UNIQUE_KEY_FLAG, MULTIPLE_KEY_FLAG, UNSIGNED_FLAG, ZEROFILL_FLAG, BINARY_FLAG, AUTO_INCREMENT_FLAG
//
// Due to the way the MySQL library handles enums, the are not supported for this type.

class db_column
{
	public:
		db_column ();
		db_column (const MYSQL_FIELD &f);
		db_column (const string &_name, const enum_field_types _type, const unsigned int _length=0, const string &_def="");
		void init_flags ();

		// name                // MYSQL_FIELD member
		string name;           // name
		enum_field_types type; // type
		unsigned int length;   // length
		string def;            // def
		// table, max_length, flags, decimals
		bool not_null, type_unsigned, zerofill, binary, auto_increment;
		//bool primary_key, unique_key, multiple_key;

		// Using modifiers:
		//   db_column col;
		//   col=db_column (foo, bar, baz).set_not_null (true).set_autoincrement (false)
#define MODIFIER(type, var)	\
		db_column &set_ ## var (type _ ## var) { var=_ ## var; return *this; }
		MODIFIER (bool, not_null)
		MODIFIER (bool, type_unsigned)
		MODIFIER (bool, zerofill)
		MODIFIER (bool, binary)
		MODIFIER (bool, auto_increment)
#undef MODIFIER

		string type_string () const;
		string mysql_spec () const;
};

ostream &operator<< (ostream &s, const db_column &c);

#endif

