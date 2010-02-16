#ifndef MIGRATOR_H_
#define MIGRATOR_H_

#include <QString>

#include "src/db/migration/MigrationFactory.h"

class Database;

/**
 * A controller for managing migrations on a database.
 *
 * Migrations should not be run directly, but only through a Migrator, which
 * will keep track of the applied migrations.
 *
 * The version numbers of all applied migrations are stored in a table
 * (#migrationsTableName), similar to Rails.
 */
class Migrator
{
	public:
		// *** Constants
		static const QString migrationsTableName, migrationsColumnName;

		// *** Construction
		Migrator (Database &database);
		virtual ~Migrator ();

		// *** Migration
		void up ();
		void down ();
		void migrate ();

		// *** Migration listing
		QList<quint64> pendingMigrations ();
		quint64 nextMigration ();

		// *** Migrations table
		quint64 currentVersion ();
		void addMigration (quint64 version);
		void removeMigration (quint64 version);
		void createMigrationsTable ();
		bool hasMigration (quint64 version);
		QList<quint64> appliedMigrations ();

	protected:
		// *** Migration
		void runMigration (quint64 version, Migration::Direction direction);

	private:
		Database &database;
		MigrationFactory factory;
};

#endif /* MIGRATOR_H_ */
