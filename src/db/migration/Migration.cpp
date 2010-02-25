#include "Migration.h"

#include "src/db/Database.h"

QString Migration::dataTypeBinary    () { return Db::Interface::DefaultInterface::dataTypeBinary    (); }
QString Migration::dataTypeBoolean   () { return Db::Interface::DefaultInterface::dataTypeBoolean   (); }
QString Migration::dataTypeDate      () { return Db::Interface::DefaultInterface::dataTypeDate      (); }
QString Migration::dataTypeDatetime  () { return Db::Interface::DefaultInterface::dataTypeDatetime  (); }
QString Migration::dataTypeDecimal   () { return Db::Interface::DefaultInterface::dataTypeDecimal   (); }
QString Migration::dataTypeFloat     () { return Db::Interface::DefaultInterface::dataTypeFloat     (); }
QString Migration::dataTypeInteger   () { return Db::Interface::DefaultInterface::dataTypeInteger   (); }
QString Migration::dataTypeString    () { return Db::Interface::DefaultInterface::dataTypeString    (); }
QString Migration::dataTypeText      () { return Db::Interface::DefaultInterface::dataTypeText      (); }
QString Migration::dataTypeTime      () { return Db::Interface::DefaultInterface::dataTypeTime      (); }
QString Migration::dataTypeTimestamp () { return Db::Interface::DefaultInterface::dataTypeTimestamp (); }
QString Migration::dataTypeCharacter () { return Db::Interface::DefaultInterface::dataTypeCharacter (); }
QString Migration::dataTypeId        () { return Db::Interface::DefaultInterface::dataTypeId        (); }

Migration::Migration (Db::Interface::DefaultInterface &interface):
	interface (interface)
{

}

Migration::~Migration ()
{
}

/** Forwards to database#transaction */
bool Migration::transaction ()
{
	return interface.transaction ();
}

/** Forwards to database#commit */
bool Migration::commit ()
{
	return interface.commit ();
}

/** Forwards to database#rollback */
bool Migration::rollback ()
{
	return interface.rollback ();
}

/** Forwards to database#executeQuery */
void Migration::executeQuery (const Db::Query &query, bool forwardOnly)
{
	interface.executeQuery (query, forwardOnly);
}

/** Forwards to database#createTable */
void Migration::createTable (const QString &name, bool skipIfExists)
{
	interface.createTable (name, skipIfExists);
}

/** Forwards to database#createTableLike */
void Migration::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	interface.createTableLike (like, name, skipIfExists);
}

/** Forwards to database#dropTable */
void Migration::dropTable (const QString &name)
{
	interface.dropTable (name);
}

/** Forwards to database#renameTable */
void Migration::renameTable (const QString &oldName, const QString &newName)
{
	interface.renameTable (oldName, newName);
}


/** Forwards to database#addColumn */
void Migration::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
{
	interface.addColumn (table, name, type, extraSpecification, skipIfExists);
}

/** Forwards to database#changeColumnType */
void Migration::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	interface.changeColumnType (table, name, type, extraSpecification);
}

/** Forwards to database#dropColumn */
void Migration::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
{
	interface.dropColumn (table, name, skipIfNotExists);
}

/** Forwards to database#renameColumn */
void Migration::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	interface.renameColumn (table, oldName, newName, type, extraSpecification);
}


/** Forwards to database#updateColumnValues */
void Migration::updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue)
{
	interface.updateColumnValues (tableName, columnName, oldValue, newValue);
}
