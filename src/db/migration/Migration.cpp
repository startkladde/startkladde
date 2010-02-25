#include "Migration.h"

#include "src/db/Database.h"

QString Migration::dataTypeBinary    () { return Db::Interface::DatabaseInterface::dataTypeBinary    (); }
QString Migration::dataTypeBoolean   () { return Db::Interface::DatabaseInterface::dataTypeBoolean   (); }
QString Migration::dataTypeDate      () { return Db::Interface::DatabaseInterface::dataTypeDate      (); }
QString Migration::dataTypeDatetime  () { return Db::Interface::DatabaseInterface::dataTypeDatetime  (); }
QString Migration::dataTypeDecimal   () { return Db::Interface::DatabaseInterface::dataTypeDecimal   (); }
QString Migration::dataTypeFloat     () { return Db::Interface::DatabaseInterface::dataTypeFloat     (); }
QString Migration::dataTypeInteger   () { return Db::Interface::DatabaseInterface::dataTypeInteger   (); }
QString Migration::dataTypeString    () { return Db::Interface::DatabaseInterface::dataTypeString    (); }
QString Migration::dataTypeText      () { return Db::Interface::DatabaseInterface::dataTypeText      (); }
QString Migration::dataTypeTime      () { return Db::Interface::DatabaseInterface::dataTypeTime      (); }
QString Migration::dataTypeTimestamp () { return Db::Interface::DatabaseInterface::dataTypeTimestamp (); }
QString Migration::dataTypeCharacter () { return Db::Interface::DatabaseInterface::dataTypeCharacter (); }
QString Migration::dataTypeId        () { return Db::Interface::DatabaseInterface::dataTypeId        (); }

Migration::Migration (Db::Interface::DatabaseInterface &databaseInterface):
	databaseInterface (databaseInterface)
{

}

Migration::~Migration ()
{
}

/** Forwards to database#transaction */
bool Migration::transaction ()
{
	return databaseInterface.transaction ();
}

/** Forwards to database#commit */
bool Migration::commit ()
{
	return databaseInterface.commit ();
}

/** Forwards to database#rollback */
bool Migration::rollback ()
{
	return databaseInterface.rollback ();
}

/** Forwards to database#prepareQuery */
QSqlQuery Migration::prepareQuery (const QString &queryString)
{
	return databaseInterface.prepareQuery (queryString);
}

/** Forwards to database#executeQuery */
QSqlQuery &Migration::executeQuery (QSqlQuery &query)
{
	return databaseInterface.executeQuery (query);
}

/** Forwards to database#executeQuery */
QSqlQuery Migration::executeQuery (const QString &queryString)
{
	return databaseInterface.executeQuery (queryString);
}

/** Forwards to database#createTable */
void Migration::createTable (const QString &name, bool skipIfExists)
{
	databaseInterface.createTable (name, skipIfExists);
}

/** Forwards to database#createTableLike */
void Migration::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	databaseInterface.createTableLike (like, name, skipIfExists);
}

/** Forwards to database#dropTable */
void Migration::dropTable (const QString &name)
{
	databaseInterface.dropTable (name);
}

/** Forwards to database#renameTable */
void Migration::renameTable (const QString &oldName, const QString &newName)
{
	databaseInterface.renameTable (oldName, newName);
}


/** Forwards to database#addColumn */
void Migration::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
{
	databaseInterface.addColumn (table, name, type, extraSpecification, skipIfExists);
}

/** Forwards to database#changeColumnType */
void Migration::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	databaseInterface.changeColumnType (table, name, type, extraSpecification);
}

/** Forwards to database#dropColumn */
void Migration::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
{
	databaseInterface.dropColumn (table, name, skipIfNotExists);
}

/** Forwards to database#renameColumn */
void Migration::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	databaseInterface.renameColumn (table, oldName, newName, type, extraSpecification);
}


/** Forwards to database#updateColumnValues */
void Migration::updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue)
{
	databaseInterface.updateColumnValues (tableName, columnName, oldValue, newValue);
}
