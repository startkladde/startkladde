#ifndef MIGRATOR_H_
#define MIGRATOR_H_

#include <QString>

#include "src/db/migration/MigrationFactory.h"

class Database;

class Migrator
{
	public:
		static const QString migrationsTableName, migrationsColumnName;

		Migrator (Database &database);
		virtual ~Migrator ();

		void up ();
		void down ();

		QString getVersion ();
		void addMigration (QString name);
		void removeMigration (QString name);

	protected:
		void applyMigration (QString name, bool up);

	private:
		Database &database;
		MigrationFactory factory;
};

#endif /* MIGRATOR_H_ */
