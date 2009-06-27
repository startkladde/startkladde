#ifndef _DbColumn_h
#define _DbColumn_h

#include <iostream>
#include <QString>

#include <mysql.h>

#include "src/text.h"


// Flags: NOT_NULL_FLAG, PRI_KEY_FLAG, UNIQUE_KEY_FLAG, MULTIPLE_KEY_FLAG, UNSIGNED_FLAG, ZEROFILL_FLAG, BINARY_FLAG, AUTO_INCREMENT_FLAG
//
// Due to the way the MySQL library handles enums, the are not supported for this type.

class DbColumn
{
	public:
		DbColumn ();
		DbColumn (const MYSQL_FIELD &f);
		DbColumn (const QString &_name, const enum_field_types _type, const unsigned int _length=0, const QString &_def="");
		void init_flags ();

		// name                // MYSQL_FIELD member
		QString name;           // name
		enum_field_types type; // type
		unsigned int length;   // length
		QString def;            // def
		// Table, max_length, flags, decimals
		bool not_null, type_unsigned, zerofill, binary, auto_increment;
		//bool primary_key, unique_key, multiple_key;

		// Using modifiers:
		//   DbColumn col;
		//   col=DbColumn (foo, bar, baz).set_not_null (true).set_autoincrement (false)
#define MODIFIER(type, var)	\
		DbColumn &set_ ## var (type _ ## var) { var=_ ## var; return *this; }
		MODIFIER (bool, not_null)
		MODIFIER (bool, type_unsigned)
		MODIFIER (bool, zerofill)
		MODIFIER (bool, binary)
		MODIFIER (bool, auto_increment)
#undef MODIFIER

		QString type_string () const;
		QString mysql_spec () const;
};

std::ostream &operator<< (std::ostream &s, const DbColumn &c);

#endif

