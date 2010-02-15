#include "MigrationFactory.h"

#include "src/db/migrations/Migration_20100214140000_initial.h"
#include "src/db/migrations/Migration_20100215172237_add_towpilot.h"

// When autogenerating the migrations list:
//   - make sure version is numeric
//   - make sure name and version are unique
//   - sort by version

MigrationFactory::MigrationFactory ()
{
}

MigrationFactory::~MigrationFactory ()
{
}



/**
 * Lists the versions of all available migrations, sorted in ascending order.
 *
 * @return a QList of migration versions
 */
QList<quint64> MigrationFactory::availableVersions ()
{
	QList<quint64> versions;

	versions << 20100214140000ll;
	versions << 20100215172237ll;

	return versions;
}

/**
 * Determines the latest available migration. This is the last entry of the
 * list returned by #availableVersions, unless no migrations exist.
 *
 * @return the latest available migration, or 0 if no migrations exist
 */
quint64 MigrationFactory::latestVersion ()
{
	QList<quint64> versions=availableVersions ();

	if (versions.empty ())
		return 0;
	else
		return versions.last ();
}



/**
 * Creates a new Migration instance of the given version.
 *
 * The caller takes ownership of the Migration.
 *
 * @param database the database to create the migration for
 * @param version the version of the migration to generate. Must have been
 *                obtained from the same MigrationFactory instance.
 * @return a newly allocated instance of Migration for the given version
 * @throw NoSuchMigrationException if there is no migration with the given
 *        version
 */
Migration *MigrationFactory::createMigration (Database &database, const quint64 version)
{
	if (version==20100214140000ll)
		return new Migration_20100214140000_initial (database);
	else if (version==20100215172237ll)
		return new Migration_20100215172237_add_towpilot (database);

	throw NoSuchMigrationException (version);
}

/**
 * Determines the name of a given migration version.
 *
 * @param version the version of the migration to determine the name of. Must
 *                have been obtained from the same MigrationFactory instance.
 * @return the name of the migration with the given version
 * @throw NoSuchMigrationException if there is no migration with the given
 *        version
 */
QString MigrationFactory::migrationName (quint64 version)
{
	if (version==20100214140000ll)
		return "initial";
	else if (version==20100215172237ll)
		return "add_towpilot";

	throw NoSuchMigrationException (version);
}
