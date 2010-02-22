#include "DatabaseInterface.h"

#include <iostream>

#include <QVariant>

#include "src/util/qString.h"

// ***************
// ** Constants **
// ***************

// Note: these values are used in migrations. If they are changed, the
// migrations should be updated to use the same values as before.
QString DatabaseInterface::dataTypeBinary    () { return "blob"            ; }
QString DatabaseInterface::dataTypeBoolean   () { return "tinyint(1)"      ; }
QString DatabaseInterface::dataTypeDate      () { return "date"            ; }
QString DatabaseInterface::dataTypeDatetime  () { return "datetime"        ; }
QString DatabaseInterface::dataTypeDecimal   () { return "decimal"         ; }
QString DatabaseInterface::dataTypeFloat     () { return "float"           ; }
QString DatabaseInterface::dataTypeInteger   () { return "int(11)"         ; }
QString DatabaseInterface::dataTypeString    () { return "varchar(255)"    ; }
QString DatabaseInterface::dataTypeText      () { return "text"            ; }
QString DatabaseInterface::dataTypeTime      () { return "time"            ; }
QString DatabaseInterface::dataTypeTimestamp () { return "datetime"        ; }
QString DatabaseInterface::dataTypeCharacter () { return "varchar(1)"      ; } // Non-Rails
QString DatabaseInterface::dataTypeId        () { return dataTypeInteger (); }


// ******************
// ** Construction **
// ******************

DatabaseInterface::DatabaseInterface ()
{
    db=QSqlDatabase::addDatabase ("QMYSQL");
}

DatabaseInterface::~DatabaseInterface ()
{
	if (db.isOpen ())
		close ();
}


// ***************************
// ** Connection management **
// ***************************

bool DatabaseInterface::open (const DatabaseInfo &dbInfo)
{
	info=dbInfo;

	std::cout << QString ("Connecting to %1@%2:%3")
		.arg (dbInfo.username, dbInfo.server, dbInfo.database) << std::endl;

    db.setHostName     (dbInfo.server  );
    db.setUserName     (dbInfo.username);
    db.setPassword     (dbInfo.password);
    db.setPort         (dbInfo.port    );
    db.setDatabaseName (dbInfo.database);

    bool result=db.open ();
	if (!result) return false;

//    QSqlQuery query (db);
//    query.prepare ("show variables like 'char%'");
//    executeQuery (query);
//
//    while (query.next())
//    {
//    	QString name=query.value(0).toString ();
//    	QString value=query.value(1).toString ();
//    	std::cout << QString ("%1=%2").arg (name).arg (value) << std::endl;
//    }

	return result;
}

void DatabaseInterface::close ()
{
	db.close ();
}


// *************
// ** Queries **
// *************

/**
 * Prepares a query with the given query string and returns it. You can then
 * add bind values to the query.
 *
 * Do not execute the query retrieved by this method directly; pass it to
 * #executeQuery for propper error handling and logging.
 *
 * This method may be removed when we have a Query class.
 */
QSqlQuery DatabaseInterface::prepareQuery (QString queryString, bool forwardOnly)
{
	QSqlQuery query (db);
	query.setForwardOnly (forwardOnly);
	query.prepare (queryString);
	return query;
}

/**
 * Executes a query, retrieved from #prepareQuery and returns a reference to
 * the query. The result data can then be read from the query.
 *
 * @param query the query to execute
 * @return a reference to query
 * @throw QueryFailedException if the query fails
 */
QSqlQuery &DatabaseInterface::executeQuery (QSqlQuery &query)
{
	if (!query.exec ())
		throw QueryFailedException (query);

//	std::cout << query.lastQuery () << std::endl;

	return query;
}

/**
 * Executes a query given by a string, with no bind values, and returns the
 * query.
 *
 * @param queryString the query to execute
 * @return the QSqlQuery executed
 * @throw QueryFailedException if the query fails
 */
QSqlQuery DatabaseInterface::executeQuery (QString queryString, bool forwardOnly)
{
	QSqlQuery query=prepareQuery (queryString, forwardOnly);
	return executeQuery (query);
}

/**
 * Executes a query retrieved from #prepareQuery and returns whether the query
 * had any result.
 *
 * For determining object existence, a COUNT(*) query may be faster (?), but
 * this can be useful for queries like 'SHOW COLUMNS LIKE ...'.
 *
 * @param query the query to execute
 * @return true if the query had a result, false if not
 * @throw QueryFailedException if the query fails
 */
bool DatabaseInterface::queryHasResult (QSqlQuery &query)
{
	executeQuery (query);
	return query.size()>0;
}

/**
 * Executes a query given by a string, with no bind values, and returns
 * whether the query had any result.
 *
 * @param queryString the query to execute
 * @return true if the query had a result, false if not
 * @throw QueryFailedException if the query fails
 * @see #queryHasResult(QSqlQuery &)
 */
bool DatabaseInterface::queryHasResult (QString queryString)
{
	QSqlQuery query=executeQuery (queryString);
	return query.size()>0;
}


// *************************
// ** Schema manipulation **
// *************************

void DatabaseInterface::createTable (const QString &name, bool skipIfExists)
{
	std::cout << QString ("Creating table %1%2")
		.arg (name, skipIfExists?" if it does not exist":"")
		<< std::endl;

	QString queryString=QString (
		"CREATE TABLE %1 %2 ("
		"id int(11) NOT NULL AUTO_INCREMENT,"
		"PRIMARY KEY (id)"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		)
		.arg (skipIfExists?"IF NOT EXISTS":"", name);

	executeQuery (queryString);
}

void DatabaseInterface::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	std::cout << QString ("Creating table %1 like %2%3")
		.arg (name, like, skipIfExists?" if it does not exist":"")
		<< std::endl;

	QString queryString=
		QString ("CREATE TABLE %1 %2 LIKE %3")
		.arg (skipIfExists?"IF NOT EXISTS":"", name, like);

	executeQuery (queryString);
}

void DatabaseInterface::dropTable (const QString &name)
{
	std::cout << QString ("Dropping table %1").arg (name) << std::endl;

	QString queryString=
		QString ("DROP TABLE %1")
		.arg (name);

	executeQuery (queryString);
}

void DatabaseInterface::renameTable (const QString &oldName, const QString &newName)
{
	std::cout << QString ("Renaming table %1 to %2").arg (oldName, newName) << std::endl;

	QString queryString=
		QString ("RENAME TABLE %1 TO %2")
		.arg (oldName, newName);

	executeQuery (queryString);
}

bool DatabaseInterface::tableExists (const QString &name)
{
	// Using addBindValue does not seem to work here
	QString queryString=
		QString ("SHOW TABLES LIKE '%1'")
		.arg (name);

	return queryHasResult (queryString);
}

void DatabaseInterface::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
{
	if (skipIfExists && columnExists (table, name))
	{
		std::cout << QString ("Skipping existing column %1.%2").arg (table, name) << std::endl;
		return;
	}

	std::cout << QString ("Adding column %1.%2").arg (table, name) << std::endl;

	QString queryString=
		QString ("ALTER TABLE %1 ADD COLUMN %2 %3 %4")
		.arg (table, name, type, extraSpecification);

	executeQuery (queryString);
}

void DatabaseInterface::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	std::cout << QString ("Changing column %1.%2 type to %3")
		.arg (table, name, type) << std::endl;

	QString queryString=
		QString ("ALTER TABLE %1 MODIFY %2 %3 %4")
		.arg (table, name, type, extraSpecification);

	executeQuery (queryString);
}

void DatabaseInterface::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
{
	if (skipIfNotExists && !columnExists (table, name))
	{
		std::cout << QString ("Skipping non-existing column %1.%2").arg (table, name) << std::endl;
		return;
	}

	std::cout << QString ("Dropping column %1.%2").arg (table, name) << std::endl;

	QString queryString=
		QString ("ALTER TABLE %1 DROP COLUMN %2")
		.arg (table, name);

	executeQuery (queryString);
}

void DatabaseInterface::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	std::cout << QString ("Renaming column %1.%2 to %3").arg (table, oldName, newName) << std::endl;

	QString queryString=
		QString ("ALTER TABLE %1 CHANGE %2 %3 %4 %5")
		.arg (table, oldName, newName, type, extraSpecification);

	executeQuery (queryString);
}

bool DatabaseInterface::columnExists (const QString &table, const QString &name)
{
	// Using addBindValue does not seem to work here
	QString queryString=
		QString ("SHOW COLUMNS FROM %1 LIKE '%2'")
		.arg (table, name);

	return queryHasResult (queryString);
}


// ******************************
// ** Generic query generation **
// ******************************

QString DatabaseInterface::selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty)
{
	// "select distinct column from table"
	QString query=QString ("SELECT DISTINCT %1 FROM %2").arg (column, table);

	// ..." where column!=''"
	if (excludeEmpty) query+=QString (" WHERE %1!=''").arg (column);
	return query;
}

QString DatabaseInterface::selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty)
{
	QStringList parts;

	foreach (QString table, tables)
		foreach (QString column, columns)
			parts << selectDistinctColumnQuery (table, column, excludeEmpty);

	return parts.join (" UNION ");
}

QString DatabaseInterface::selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty)
{
	return selectDistinctColumnQuery (tables, QStringList (column), excludeEmpty);
}

QString DatabaseInterface::selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty)
{
	return selectDistinctColumnQuery (QStringList (table), columns, excludeEmpty);
}

// *******************************
// ** Generic data manipulation **
// *******************************

void DatabaseInterface::updateColumnValues (const QString &tableName, const QString &columnName,
	const QVariant &oldValue, const QVariant &newValue)
{
	QString queryString=QString ("UPDATE %1 SET %2=? WHERE %2=?")
		.arg (tableName, columnName);

	QSqlQuery query=prepareQuery (queryString);
	query.addBindValue (newValue);
	query.addBindValue (oldValue);

	executeQuery (query);
}

QStringList DatabaseInterface::listStrings (QString queryString)
{
	QSqlQuery query=executeQuery (queryString);

	QStringList stringList;

	while (query.next ())
		stringList.append (query.value (0).toString ());

	return stringList;
}
