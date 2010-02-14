#include "MigrationFactory.h"

#include "src/db/migrations/Migration_20100214_140000_initial.h"

MigrationFactory::MigrationFactory (Database &database):
	database (database)
{
}

MigrationFactory::~MigrationFactory ()
{
}

QStringList MigrationFactory::migrationNames ()
{
	QStringList names;

	names << "20100214_140000_initial";

	return names;
}

Migration *MigrationFactory::createMigration (const QString &name)
{
	if (name=="20100214_140000_initial")
		return new Migration_20100214_140000_initial (database);

	return NULL;
}
