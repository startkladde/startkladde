#ifndef MIGRATIONFACTORY_H_
#define MIGRATIONFACTORY_H_

#include <QStringList>

#include "src/db/migration/Migration.h"

class Database;

class MigrationFactory
{
	public:
		MigrationFactory (Database &database);
		virtual ~MigrationFactory ();

		QStringList migrationNames ();
		Migration *createMigration (const QString &name);

	private:
		Database &database;
};

#endif /* MIGRATIONFACTORY_H_ */
