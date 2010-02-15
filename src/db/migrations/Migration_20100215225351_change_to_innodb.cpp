#include "Migration_20100215225351_change_to_innodb.h"

#include "src/db/Database.h"

Migration_20100215225351_change_to_innodb::Migration_20100215225351_change_to_innodb (Database &database):
	Migration (database)
{
}

Migration_20100215225351_change_to_innodb::~Migration_20100215225351_change_to_innodb ()
{
}

void Migration_20100215225351_change_to_innodb::up ()
{
	QStringList tables;
	tables << "person" << "person_temp";
	tables << "flugzeug" << "flugzeug_temp";
	tables << "flug" << "flug_temp";
	tables << "user";

	foreach (QString table, tables)
		database.executeQuery (
			QString ("ALTER TABLE %1 ENGINE=InnoDB")
			.arg (table)
		);
}

void Migration_20100215225351_change_to_innodb::down ()
{
	// Don't change back
}
