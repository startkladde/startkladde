#include "Migration_20100215215320_convert_to_utf8.h"

#include "src/db/Database.h"

Migration_20100215215320_convert_to_utf8::Migration_20100215215320_convert_to_utf8 (Database &database):
	Migration (database)
{
}

Migration_20100215215320_convert_to_utf8::~Migration_20100215215320_convert_to_utf8 ()
{
}

void Migration_20100215215320_convert_to_utf8::up ()
{
	QStringList tables;
	tables << "person" << "person_temp";
	tables << "flugzeug" << "flugzeug_temp";
	tables << "flug" << "flug_temp";
	tables << "user";

	foreach (QString table, tables)
		database.executeQuery (
			QString ("ALTER TABLE %1 CONVERT TO CHARACTER SET utf8 COLLATE utf8_unicode_ci")
			.arg (table)
		);
}

void Migration_20100215215320_convert_to_utf8::down ()
{
	// Don't change back
}
