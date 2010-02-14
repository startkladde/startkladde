#include "Migrator.h"

#include <iostream>

#include "src/text.h"
#include "src/db/Database.h"

// If this is changed, things will break, like, horribly.
const QString Migrator::migrationsTableName="schema_migrations";

Migrator::Migrator (Database &database):
	database (database)
{
}

Migrator::~Migrator ()
{
}

void Migrator::up ()
{
	std::cout << "migrator up" << std::endl;
	std::cout << QString ("version is %1").arg (getVersion ()) << std::endl;
//	database.setVersion ("up");
	std::cout << QString ("version is %1").arg (getVersion ()) << std::endl;
}

void Migrator::down ()
{
	std::cout << "migrator down" << std::endl;
	std::cout << QString ("version is %1").arg (getVersion ()) << std::endl;
//	database.setVersion ("down");
	std::cout << QString ("version is %1").arg (getVersion ()) << std::endl;
}

/**
 * Determines the version of the database
 *
 * @return a string containing the version, or an empty string if the version
 *         table does not exist or is empty
 */
QString Migrator::getVersion ()
{
	if (database.tableExists (migrationsTableName))
	{
		QSqlQuery query=database.executeQuery (
			QString ("SELECT version FROM %1 ORDER BY version DESC LIMIT 1")
			.arg (migrationsTableName)
		);

		if (query.next ())
			return query.value (0).toString ();
		else
			return "";
	}
	else
	{
		return "";
	}
}

//void Database::addMigration (QString version)
//{
//CREATE TABLE  `akaportal`.`schema_migrations` (
//  `version` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
//  UNIQUE KEY `unique_schema_migrations` (`version`)
//) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci
////	if (!tableExists ("version"))
////		executeQuery ("CREATE TABLE version (version VARCHAR(255) NOT NULL PRIMARY KEY)");
////
////	try
////	{
////		db.transaction ();
////		executeQuery ("DELETE FROM version");
////
////		QSqlQuery query (db);
////		query.prepare ("INSERT INTO version (version) VALUES (?)");
////		query.addBindValue (version);
////		executeQuery (query);
////
////		db.commit ();
////	}
////	catch (...)
////	{
////		db.rollback ();
////		throw;
////	}
//}
