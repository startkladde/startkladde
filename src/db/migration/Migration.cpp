#include "Migration.h"

#include "src/db/Database.h"

const QString Migration::dataTypeBinary    = Database::dataTypeBinary   ;
const QString Migration::dataTypeBoolean   = Database::dataTypeBoolean  ;
const QString Migration::dataTypeDate      = Database::dataTypeDate     ;
const QString Migration::dataTypeDatetime  = Database::dataTypeDatetime ;
const QString Migration::dataTypeDecimal   = Database::dataTypeDecimal  ;
const QString Migration::dataTypeFloat     = Database::dataTypeFloat    ;
const QString Migration::dataTypeInteger   = Database::dataTypeInteger  ;
const QString Migration::dataTypeString    = Database::dataTypeString   ;
const QString Migration::dataTypeText      = Database::dataTypeText     ;
const QString Migration::dataTypeTime      = Database::dataTypeTime     ;
const QString Migration::dataTypeTimestamp = Database::dataTypeTimestamp;
const QString Migration::dataTypeCharacter = Database::dataTypeCharacter;
const QString Migration::dataTypeId        = Database::dataTypeId       ;

Migration::Migration (Database &database):
	database (database)
{

}

Migration::~Migration ()
{
}

bool Migration::transaction ()
{
	return database.transaction ();
}

bool Migration::commit ()
{
	return database.commit ();
}

bool Migration::rollback ()
{
	return database.rollback ();
}

QSqlQuery Migration::prepareQuery (const QString &queryString)
{
	return database.prepareQuery (queryString);
}

QSqlQuery &Migration::executeQuery (QSqlQuery &query)
{
	return database.executeQuery (query);
}

QSqlQuery Migration::executeQuery (const QString &queryString)
{
	return database.executeQuery (queryString);
}

void Migration::createTable (const QString &name, bool skipIfExists)
{
	database.createTable (name, skipIfExists);
}

void Migration::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	database.createTableLike (like, name, skipIfExists);
}

void Migration::dropTable (const QString &name)
{
	database.dropTable (name);
}

void Migration::renameTable (const QString &oldName, const QString &newName)
{
	database.renameTable (oldName, newName);
}


void Migration::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
{
	database.addColumn (table, name, type, extraSpecification, skipIfExists);
}

void Migration::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	database.changeColumnType (table, name, type, extraSpecification);
}

void Migration::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
{
	database.dropColumn (table, name, skipIfNotExists);
}

void Migration::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	database.renameColumn (table, oldName, newName, type, extraSpecification);
}


void Migration::updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue)
{
	database.updateColumnValues (tableName, columnName, oldValue, newValue);
}
