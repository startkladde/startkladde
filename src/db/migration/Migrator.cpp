#include "Migrator.h"

#include <iostream>

#include "src/db/interface/DatabaseInterface.h"
#include "src/db/migration/MigrationFactory.h"
#include "src/db/schema/CurrentSchema.h"
#include "src/util/qString.h"

// ***************
// ** Constants **
// ***************

// If this is changed, things will break, like, horribly.
const QString Migrator::migrationsTableName="schema_migrations";
const QString Migrator::migrationsColumnName="version";

/**
 * Creates a Migrator for the given databaseInterface
 *
 * @param databaseInterface the databaseInterface to access
 */
Migrator::Migrator (Db::Interface::DatabaseInterface &databaseInterface):
	databaseInterface (databaseInterface),
	factory (new MigrationFactory ())
{
}

Migrator::~Migrator ()
{
	delete factory;
}


// ****************
// ** Migrations **
// ****************

void Migrator::runMigration (quint64 version, Migration::Direction direction)
{
	Migration *migration=NULL;
	try
	{
		migration=factory->createMigration (databaseInterface, version);
		QString name=factory->migrationName (version);

		switch (direction)
		{
			case Migration::dirUp:
				std::cout << "== Applying: " << name << " " << QString (79-14-name.length (), '=') << std::endl;
				migration->up ();
				addMigration (version);
				break;
			case Migration::dirDown:
				std::cout << "== Reverting: " << name << " " << QString (79-15-name.length (), '=') << std::endl;
				migration->down ();
				removeMigration (version);
				break;
		}

		std::cout << "== Version is now " << currentVersion () << " " << QString (79-19-14, '=') << std::endl << std::endl;

		delete migration;
	}
	catch (...)
	{
		delete migration;
		throw;
	}
}

/** Migrates one step up */
void Migrator::up ()
{
	quint64 version=nextMigration ();

	if (version==0)
	{
		std::cout << "Already current" << std::endl;
		return;
	}

	runMigration (version, Migration::dirUp);
}

/** Migrates one step down */
void Migrator::down ()
{
	quint64 version=currentVersion ();

	if (version==0) return;

	runMigration (currentVersion (), Migration::dirDown);
}

/** Migrates to the latest version (runs pending migrations) */
void Migrator::migrate ()
{
	foreach (quint64 version, pendingMigrations ())
		runMigration (version, Migration::dirUp);
}


// ************
// ** Schema **
// ************

void Migrator::loadSchema ()
{
	std::cout << "== Loading schema =============================================================" << std::endl;

	CurrentSchema schema (databaseInterface);

	schema.up ();
	assumeMigrated (schema.getVersions ());

	std::cout << "== Version is now " << currentVersion () << " " << QString (79-19-14, '=') << std::endl << std::endl;
}

void Migrator::drop ()
{
	QString databaseName=databaseInterface.getInfo ().database;
	databaseInterface.executeQuery (QString ("DROP DATABASE %1").arg (databaseName));
}

void Migrator::create ()
{
	QString databaseName=databaseInterface.getInfo ().database;
	databaseInterface.executeQuery (QString ("CREATE DATABASE %1").arg (databaseName));
}

void Migrator::clear ()
{
	drop ();
	create ();
}

void Migrator::reset ()
{
	drop ();
	create ();
	loadSchema ();
}

// ***********************
// ** Migration listing **
// ***********************

QList<quint64> Migrator::pendingMigrations ()
{
	QList<quint64> availableMigrations=factory->availableVersions ();
	QList<quint64> appliedMigrations=this->appliedMigrations ();

	QList<quint64> pending;
	foreach (quint64 version, availableMigrations)
		if (!appliedMigrations.contains (version))
			pending << version;

	return pending;
}

quint64 Migrator::nextMigration ()
{
	QList<quint64> availableMigrations=factory->availableVersions ();
	QList<quint64> appliedMigrations=this->appliedMigrations ();

	foreach (quint64 version, availableMigrations)
		if (!appliedMigrations.contains (version))
			return version;

	return 0;
}


// **********************
// ** Migrations table **
// **********************

/**
 * Determines the version of the database (the version of the lates migration)
 *
 * @return the version, or an 0 if the version table does not exist or is empty
 */
quint64 Migrator::currentVersion ()
{
	if (!databaseInterface.tableExists (migrationsTableName)) return 0;

	QSqlQuery query=databaseInterface.executeQuery (
		QString ("SELECT %2 FROM %1 ORDER BY %2 DESC LIMIT 1")
		.arg (migrationsTableName, migrationsColumnName)
	);

	if (query.next ())
		return query.value (0).toLongLong ();
	else
		return 0;
}

void Migrator::createMigrationsTable ()
{
	databaseInterface.executeQuery (
		QString (
			"CREATE TABLE %1 (%2 VARCHAR(255) NOT NULL PRIMARY KEY)"
			" ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		).arg (migrationsTableName, migrationsColumnName)
	);
}

bool Migrator::hasMigration (quint64 version)
{
	QSqlQuery query=databaseInterface.prepareQuery (
			QString ("SELECT %2 FROM %1 WHERE %2=?")
			.arg (migrationsTableName, migrationsColumnName)
			);
	query.addBindValue (version);

	return databaseInterface.queryHasResult (query);
}

void Migrator::addMigration (quint64 version)
{
	// If the migrations table does not exist, create it
	if (!databaseInterface.tableExists (migrationsTableName)) createMigrationsTable ();

	// If the migration name is already present in the migrations table, return
	if (hasMigration (version)) return;

	// Add the migration name to the migrations table
	QSqlQuery query=databaseInterface.prepareQuery (
			QString ("INSERT INTO %1 (%2) VALUES (?)")
			.arg (migrationsTableName, migrationsColumnName)
			);
	query.addBindValue (version);
	databaseInterface.executeQuery (query);
}

void Migrator::removeMigration (quint64 version)
{
	// If the migrations table does not exist, return
	if (!databaseInterface.tableExists (migrationsTableName)) return;

	// Remove the migration name from the migrations table
	QSqlQuery query=databaseInterface.prepareQuery (
			QString ("DELETE FROM %1 where %2=?")
			.arg (migrationsTableName, migrationsColumnName)
			);
	query.addBindValue (version);

	databaseInterface.executeQuery (query);
}

QList<quint64> Migrator::appliedMigrations ()
{
	QList<quint64> migrations;

	if (!databaseInterface.tableExists (migrationsTableName)) return migrations;

	QSqlQuery query=databaseInterface.prepareQuery (
			QString ("SELECT %2 FROM %1")
			.arg (migrationsTableName, migrationsColumnName)
			);

	databaseInterface.executeQuery (query);

	while (query.next ())
		migrations << query.value (0).toLongLong ();

	return migrations;
}

void Migrator::assumeMigrated (QList<quint64> versions)
{
	// If the migrations table does not exist, create it
	if (!databaseInterface.tableExists (migrationsTableName)) createMigrationsTable ();

	// Remove all migrations
	databaseInterface.executeQuery (QString ("DELETE FROM %1").arg (migrationsTableName));

	// Add all migrations
	foreach (quint64 version, versions)
	{
		QSqlQuery query=databaseInterface.prepareQuery (
			QString ("INSERT INTO %1 (%2) VALUES (?)")
			.arg (migrationsTableName, migrationsColumnName)
			);

		query.addBindValue (version);
		databaseInterface.executeQuery (query);
	}
}
