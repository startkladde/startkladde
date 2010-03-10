#include "Migrator.h"

#include <iostream>

#include <QSharedPointer>

#include "src/db/interface/Interface.h"
#include "src/db/migration/MigrationFactory.h"
#include "src/db/schema/CurrentSchema.h"
#include "src/util/qString.h"
#include "src/db/result/Result.h"
#include "src/concurrent/monitor/OperationMonitor.h"

// ***************
// ** Constants **
// ***************

// If this is changed, things will break, like, horribly.
const QString Migrator::migrationsTableName="schema_migrations";
const QString Migrator::migrationsColumnName="version";

/**
 * Creates a Migrator for the given Interface
 *
 * @param interface the Interface to access
 */
Migrator::Migrator (Interface &interface):
	interface (interface),
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

void Migrator::runMigration (quint64 version, Migration::Direction direction, OperationMonitorInterface monitor)
{
	Migration *migration=NULL;
	try
	{
		migration=factory->createMigration (interface, version);
		QString name=factory->migrationName (version);

		switch (direction)
		{
			case Migration::dirUp:
				std::cout << "== Applying: " << name << " " << QString (79-14-name.length (), '=') << std::endl;
				monitor.status (QString ("Migration anwenden: %1").arg (name));
				migration->up ();
				addMigration (version);
				break;
			case Migration::dirDown:
				std::cout << "== Reverting: " << name << " " << QString (79-15-name.length (), '=') << std::endl;
				monitor.status (QString ("Migration rückgängig machen: %1").arg (name));
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
void Migrator::migrate (OperationMonitorInterface monitor)
{
	QList<quint64> versions=pendingMigrations ();

	int progress=0, maxProgress=pendingMigrations ().size ();

	// TODO better progress reporting interface
	monitor.progress (progress, maxProgress);
	foreach (quint64 version, versions)
	{
		runMigration (version, Migration::dirUp, monitor);
		++progress;
		monitor.progress (progress, maxProgress);
	}
}


// ************
// ** Schema **
// ************

void Migrator::loadSchema (OperationMonitorInterface monitor)
{
	std::cout << "== Loading schema =============================================================" << std::endl;

	CurrentSchema schema (interface);

	monitor.status ("Schema laden");
	schema.up (monitor);

	monitor.status ("Version speichern");
	assumeMigrated (schema.getVersions ());

	std::cout << "== Version is now " << currentVersion () << " " << QString (79-19-14, '=') << std::endl << std::endl;
}

void Migrator::drop ()
{
	// TODO create and use db method
	QString databaseName=interface.getInfo ().database;
	interface.executeQuery (QString ("DROP DATABASE %1").arg (databaseName));
}

void Migrator::create ()
{
	// TODO use db method
	QString databaseName=interface.getInfo ().database;
	interface.executeQuery (QString ("CREATE DATABASE %1").arg (databaseName));
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

quint64 Migrator::latestVersion ()
{
	return MigrationFactory ().latestVersion ();
}

bool Migrator::isCurrent (OperationMonitorInterface monitor)
{
	monitor.status (utf8 ("Datenbankversion prüfen"));

	// Use nextMigration, not currentVersion, so it works with gaps
	return nextMigration ()==0;
}

bool Migrator::isEmpty (OperationMonitorInterface monitor)
{
	monitor.status (utf8 ("Datenbank prüfen"));
	return !interface.tableExists ();
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
	if (!interface.tableExists (migrationsTableName)) return 0;

	Query query=Query ("SELECT %2 FROM %1 ORDER BY %2 DESC LIMIT 1")
		.arg (migrationsTableName, migrationsColumnName);

	QSharedPointer<Result> result=interface.executeQueryResult (query);

	// TODO getValue query
	if (result->next ())
		return result->value (0).toLongLong ();
	else
		return 0;
}

void Migrator::createMigrationsTable ()
{
	interface.executeQuery (
		QString (
			"CREATE TABLE %1 (%2 VARCHAR(255) NOT NULL PRIMARY KEY)"
			" ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		).arg (migrationsTableName, migrationsColumnName)
	);
}

bool Migrator::hasMigration (quint64 version)
{
	return interface.queryHasResult (
		Query ("SELECT %2 FROM %1 WHERE %2=?")
		.arg (migrationsTableName, migrationsColumnName)
		.bind (version)
		);
}

void Migrator::addMigration (quint64 version)
{
	// If the migrations table does not exist, create it
	if (!interface.tableExists (migrationsTableName)) createMigrationsTable ();

	// If the migration name is already present in the migrations table, return
	if (hasMigration (version)) return;

	// Add the migration name to the migrations table
	interface.executeQuery (
		Query ("INSERT INTO %1 (%2) VALUES (?)")
		.arg (migrationsTableName, migrationsColumnName)
		.bind (version)
	);
}

void Migrator::removeMigration (quint64 version)
{
	// If the migrations table does not exist, return
	if (!interface.tableExists (migrationsTableName)) return;

	// Remove the migration name from the migrations table
	interface.executeQuery (
		Query ("DELETE FROM %1 where %2=?")
			.arg (migrationsTableName, migrationsColumnName)
			.bind (version)
	);
}

QList<quint64> Migrator::appliedMigrations ()
{
	QList<quint64> migrations;

	if (!interface.tableExists (migrationsTableName)) return migrations;

	Query query=Query::selectDistinctColumns (
		migrationsTableName, migrationsColumnName);

	QSharedPointer<Result> result=interface.executeQueryResult (query);

	// TODO listValues method
	while (result->next ())
		migrations.append (result->value (0).toLongLong ());

	return migrations;
}

void Migrator::assumeMigrated (QList<quint64> versions)
{
	// If the migrations table does not exist, create it
	if (!interface.tableExists (migrationsTableName)) createMigrationsTable ();

	// Remove all migrations
	interface.executeQuery (QString ("DELETE FROM %1").arg (migrationsTableName));

	// Add all migrations
	// TODO one query
	foreach (quint64 version, versions)
	{
		Query query=Query ("INSERT INTO %1 (%2) VALUES (?)")
			.arg (migrationsTableName, migrationsColumnName)
			.bind (version);

		interface.executeQuery (query);
	}
}
