#include "Migrator.h"

#include <iostream>

#include "src/text.h"
#include "src/db/Database.h"
#include "src/db/migration/MigrationFactory.h"

// If this is changed, things will break, like, horribly.
const QString Migrator::migrationsTableName="schema_migrations";
const QString Migrator::migrationsColumnName="version";

Migrator::Migrator (Database &database):
	database (database), factory (database)
{
}

Migrator::~Migrator ()
{
}

void Migrator::applyMigration (QString name, bool up)
{
	Migration *migration=NULL;

	try
	{
		migration=factory.createMigration (name);

		if (up)
		{
			std::cout << "Migrating up " << name << std::endl;
			migration->up ();
			addMigration (name);
		}
		else
		{
			std::cout << "Migrating down " << name << std::endl;
			migration->down ();
			removeMigration (name);
		}
		std::cout << "Version is now " << getVersion () << std::endl;

		delete migration;
	}
	catch (...)
	{
		delete migration;
		throw;
	}
}

void Migrator::up ()
{
	applyMigration (factory.latest (), true);
}

void Migrator::down ()
{
	applyMigration (factory.latest (), false);
}

/**
 * Determines the version (the name of the lates migration) of the database
 *
 * @return a string containing the version, or an empty string if the version
 *         table does not exist or is empty
 */
QString Migrator::getVersion ()
{
	if (!database.tableExists (migrationsTableName)) return "";

	QSqlQuery query=database.executeQuery (
		QString ("SELECT %2 FROM %1 ORDER BY %2 DESC LIMIT 1")
		.arg (migrationsTableName, migrationsColumnName)
	);

	if (query.next ())
		return query.value (0).toString ();
	else
		return "";
}

void Migrator::addMigration (QString name)
{
	// If the migrations table does not exist, create it
	if (!database.tableExists (migrationsTableName))
	{
		database.executeQuery (
			QString ("CREATE TABLE %1 (%2 VARCHAR(255) NOT NULL PRIMARY KEY) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci")
			.arg (migrationsTableName, migrationsColumnName)
		);
	}

	// If the migration name is already present in the migrations table, return
	QSqlQuery query=database.prepareQuery (
			QString ("SELECT %2 FROM %1 WHERE %2=?")
			.arg (migrationsTableName, migrationsColumnName)
			);
	query.addBindValue (name);
	if (database.queryHasResult (query)) return;

	// Add the migration name to the migrations table
	query=database.prepareQuery (
			QString ("INSERT INTO %1 (%2) VALUES (?)")
			.arg (migrationsTableName, migrationsColumnName)
			);
	query.addBindValue (name);
	database.executeQuery (query);
}

void Migrator::removeMigration (QString name)
{
	// If the migrations table does not exist, return
	if (!database.tableExists (migrationsTableName)) return;

	// Remove the migration name from the migrations table
	QSqlQuery query=database.prepareQuery (
			QString ("DELETE FROM %1 where %2=?")
			.arg (migrationsTableName, migrationsColumnName)
			);
	query.addBindValue (name);

	database.executeQuery (query);
}
