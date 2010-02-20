#ifndef MIGRATION_H_
#define MIGRATION_H_

#include "QString"
#include "QVariant"
#include "QSqlQuery"

class Database;

/**
 * A new migration is created by creating the appropriate class in
 * src/db/migrations/. The migration is automatically included into the build
 * and recognized by MigrationFactory.
 *
 * A new migration class can be created automatically by using the script
 * script/generate_migration.rb (see documentation).
 *
 * Migrations are supposed to be called only through Migrator, which keeps track
 * of which migrations have already been applied (see documentation).
 *
 * Note that we forward a lot of Database methods (and constants) to database
 * instead of having Migration implementations access the database directly.
 * This is done in order to avoid a dependency of the individual migrations on
 * the Database, which would necessitate recompiling all migrations when the
 * database interface changes, even if the migrations are not affected by the
 * change.
 *
 * Another potential use of this is that this can be used to change the
 * behavior of the database methods invoked by the migrations, for example to
 * add logging or accumulate changes before executing them.
 *
 * Notes for implementations:
 *   - do not include Database.h. Use the forwarder methods provided by this
 *     class instead. Add forwarder methods if required.
 */
class Migration
{
	public:
		// *** Types
		enum Direction { dirUp, dirDown };

		// *** Constants
    	static const QString dataTypeBinary;
    	static const QString dataTypeBoolean;
    	static const QString dataTypeDate;
    	static const QString dataTypeDatetime;
    	static const QString dataTypeDecimal;
    	static const QString dataTypeFloat;
    	static const QString dataTypeInteger;
    	static const QString dataTypeString;
    	static const QString dataTypeString16; // Non-rails
    	static const QString dataTypeText;
    	static const QString dataTypeTime;
    	static const QString dataTypeTimestamp;
    	static const QString dataTypeCharacter; // Non-Rails
    	static const QString dataTypeId;


		Migration (Database &database);
		virtual ~Migration ();

		virtual void up ()=0;
		virtual void down ()=0;

	protected:
		bool transaction ();
		bool commit ();
		bool rollback ();

		QSqlQuery prepareQuery (const QString &queryString);
		QSqlQuery &executeQuery (QSqlQuery &query);
		QSqlQuery executeQuery (const QString &queryString);

		void createTable (const QString &name, bool skipIfExists=false);
		void createTableLike (const QString &like, const QString &name, bool skipIfExists=false);
		void dropTable (const QString &name);
		void renameTable (const QString &oldName, const QString &newName);

		void addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="", bool skipIfExists=false);
		void changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="");
		void dropColumn (const QString &table, const QString &name, bool skipIfNotExists=false);
		void renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification="");

		void updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue);

	private:
		Database &database;
};

#endif /* MIGRATION_H_ */
