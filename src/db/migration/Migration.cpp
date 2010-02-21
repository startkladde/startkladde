#include "Migration.h"

#include "src/db/Database.h"

QString Migration::dataTypeBinary    () { return Database::dataTypeBinary   ; }
QString Migration::dataTypeBoolean   () { return Database::dataTypeBoolean  ; }
QString Migration::dataTypeDate      () { return Database::dataTypeDate     ; }
QString Migration::dataTypeDatetime  () { return Database::dataTypeDatetime ; }
QString Migration::dataTypeDecimal   () { return Database::dataTypeDecimal  ; }
QString Migration::dataTypeFloat     () { return Database::dataTypeFloat    ; }
QString Migration::dataTypeInteger   () { return Database::dataTypeInteger  ; }
QString Migration::dataTypeString    () { return Database::dataTypeString   ; }
QString Migration::dataTypeText      () { return Database::dataTypeText     ; }
QString Migration::dataTypeTime      () { return Database::dataTypeTime     ; }
QString Migration::dataTypeTimestamp () { return Database::dataTypeTimestamp; }
QString Migration::dataTypeCharacter () { return Database::dataTypeCharacter; }
QString Migration::dataTypeId        () { return Database::dataTypeId       ; }

Migration::Migration (Database &database):
	database (database)
{

}

Migration::~Migration ()
{
}

/** Forwards to database#transaction */
bool Migration::transaction ()
{
	return database.transaction ();
}

/** Forwards to database#commit */
bool Migration::commit ()
{
	return database.commit ();
}

/** Forwards to database#rollback */
bool Migration::rollback ()
{
	return database.rollback ();
}

/** Forwards to database#prepareQuery */
QSqlQuery Migration::prepareQuery (const QString &queryString)
{
	return database.prepareQuery (queryString);
}

/** Forwards to database#executeQuery */
QSqlQuery &Migration::executeQuery (QSqlQuery &query)
{
	return database.executeQuery (query);
}

/** Forwards to database#executeQuery */
QSqlQuery Migration::executeQuery (const QString &queryString)
{
	return database.executeQuery (queryString);
}

/** Forwards to database#createTable */
void Migration::createTable (const QString &name, bool skipIfExists)
{
	database.createTable (name, skipIfExists);
}

/** Forwards to database#createTableLike */
void Migration::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	database.createTableLike (like, name, skipIfExists);
}

/** Forwards to database#dropTable */
void Migration::dropTable (const QString &name)
{
	database.dropTable (name);
}

/** Forwards to database#renameTable */
void Migration::renameTable (const QString &oldName, const QString &newName)
{
	database.renameTable (oldName, newName);
}


/** Forwards to database#addColumn */
void Migration::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
{
	database.addColumn (table, name, type, extraSpecification, skipIfExists);
}

/** Forwards to database#changeColumnType */
void Migration::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	database.changeColumnType (table, name, type, extraSpecification);
}

/** Forwards to database#dropColumn */
void Migration::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
{
	database.dropColumn (table, name, skipIfNotExists);
}

/** Forwards to database#renameColumn */
void Migration::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	database.renameColumn (table, oldName, newName, type, extraSpecification);
}


/** Forwards to database#updateColumnValues */
void Migration::updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue)
{
	database.updateColumnValues (tableName, columnName, oldValue, newValue);
}
