#ifndef MIGRATIONFACTORY_H_
#define MIGRATIONFACTORY_H_

#include <QStringList>

namespace Db { namespace Interface { class DatabaseInterface; } }
class Migration;

/**
 * Determines available migrations, creates migrations and determines
 * migration information.
 */
class MigrationFactory
{
	public:
		class NoSuchMigrationException: public std::exception
		{
			public:
				NoSuchMigrationException (quint64 version): version (version) {}
				quint64 version;
		};

		MigrationFactory ();
		virtual ~MigrationFactory ();

		QList<quint64> availableVersions ();
		quint64 latestVersion ();

		Migration *createMigration (Db::Interface::DatabaseInterface &databaseInterface, quint64 version);
		QString migrationName (quint64 version);
};

#endif
