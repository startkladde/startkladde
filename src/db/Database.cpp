/*
 * Next:
 *   - schema loading
 *   - integrate schema loading/database migration into GUI
 *
 * Short term plan:
 *   - Test the migrations: make sure the old version and current sk_web work
 *     with "initial"
 *   - Fixtures
 *     - SQL dump or data file (CSV/YAML)?
 *     - C++ or Ruby?
 *   - Migration: distinguish between "old" and "empty"
 *   - integrate migration into the gui
 *   - Standardize enum handling: store the database value internally and have
 *     an "unknown" type (instead of "none")
 *     - this should also allow preserving unknown types in the database
 *   - add "object used" to Database
 *   - add ping to Database
 *   - generate string lists from other data instead of explicit query (but still
 *     cache explicitly) note that we still have to query for accounting notes and
 *     airfields because we don't get a complete flight list
 *   - fix error reporting (db.lastError (), e. g. delete objects)
 *   - make sure "", 0 and NULL are read correctly
 *
 * Improvements:
 *   - split this class into parts (generic, orm, specific)
 *   - move (static) methods like selectDistinctColumnQuery to QueryGenerator (but see below)
 *   - make a class Query, a la Query ("select * from foo where id=?") << 42
 *   - allow specifying an "exclude" value to selectDistinctColumnQuery (requires
 *     Query class)
 *   - move selectDistinctColumnQuery to Query
 *   - move specialized queries generation to model classes (e. g. flight prepared)
 *   - maybe we would like to select additional columns, like
 *     (landing_time-takeoff_time as duration) for some conditions
 *   - Flight should be an entity
 *   - Allow tables without id column (e. g. habtm join tables), but tables
 *     must have at least one column
 *   - dbTableName as static constant instead of function
 *   - make sure (using a dump of an old db) that the migration path is OK
 *   - Use low level methods in ORM methods (better: split class)
 *   - Remove code multiplication in Models (SQL interface)
 *   - change flight mode (and towflight mode) column type
 *
 * Medium term plan:
 *   - add some abstraction to the query list generation
 *   - add support for sqlite
 *   - add indexes
 *   - add foreign key constraints
 *   - database checks (?)
 *     - show what is wrong/will be fixed
 *
 * Long term plan:
 *   - Shold there be a thread safe database class?
 *   - use a memory SQLite for local storage (datastorage functionality)
 *   - merge data storage and database methods, so we can use a (local)
 *     database directly (w/o local cache)
 *       - we'd probably have abstract DataStorage and inherited Database and
 *         CachingDataStorage which uses ad Database for access and one (in
 *         memory) for local caching
 *       - question: can we afford using a query on a memory database e. g.
 *         for list of first names rather than maintaining the list explicitly?
 *         should be fast enough (does sqlite memory table have index?)
 *         airfields are more interesting, b/c there are many flights
 *   - local disk caching
 */

#include "Database.h"

#include <iostream>
#include <cassert>

#include <QDateTime>

#include "src/db/DatabaseInfo.h"
#include "src/config/Options.h"
#include "src/text.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"


/*
 * Here's an idea for specifying conditions, or parts thereof:
 * Condition ("foo=? and bar=?") << 42 << "baz";
 */

// ***************
// ** Constants **
// ***************

// Note: these values are used in migrations. If they are changed, the
// migrations should be updated to use the same values as before.
const QString Database::dataTypeBinary    = "blob";
const QString Database::dataTypeBoolean   = "tinyint(1)";
const QString Database::dataTypeDate      = "date";
const QString Database::dataTypeDatetime  = "datetime";
const QString Database::dataTypeDecimal   = "decimal";
const QString Database::dataTypeFloat     = "float";
const QString Database::dataTypeInteger   = "int(11)";
const QString Database::dataTypeString    = "varchar(255)";
//const QString Database::dataTypeString16  = "varchar(16)";
const QString Database::dataTypeText      = "text";
const QString Database::dataTypeTime      = "time";
const QString Database::dataTypeTimestamp = "datetime";
const QString Database::dataTypeCharacter = "varchar(1)"; // Non-Rails
const QString Database::dataTypeId        = dataTypeInteger;


// ******************
// ** Construction **
// ******************

Database::Database ()
{
    db=QSqlDatabase::addDatabase ("QMYSQL");
}

bool Database::open (const DatabaseInfo &dbInfo)
{
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

void Database::close ()
{
	db.close ();
}

Database::~Database()
{
	if (db.isOpen ())
		close ();
}


// *************
// ** Queries **
// *************

/**
 * Do not execute the query retrieved by this method directly; pass it to
 * #executeQuery for propper error handling and logging.
 *
 * @param queryString
 * @return
 */
QSqlQuery Database::prepareQuery (QString queryString)
{
	QSqlQuery query (db);
	query.prepare (queryString);
	return queryString;
}

QSqlQuery &Database::executeQuery (QSqlQuery &query)
{
	if (!query.exec ())
		throw QueryFailedException (query);

//	std::cout << query.lastQuery () << std::endl;

	return query;
}

QSqlQuery Database::executeQuery (QString queryString)
{
	QSqlQuery query (db);
	query.prepare (queryString);
	return executeQuery (query);
}


// ******************
// ** Very generic **
// ******************

QStringList Database::listStrings (QSqlQuery query)
{
	QStringList stringList;

	while (query.next ())
		stringList.append (query.value (0).toString ());

	return stringList;
}

QStringList Database::listStrings (QString query)
{
	QSqlQuery q (query, db);
	executeQuery (q);
	return listStrings (q);
}

QString Database::selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty)
{
	// "select distinct column from table"
	QString query=QString ("SELECT DISTINCT %1 FROM %2").arg (column, table);

	// ..." where column!=''"
	if (excludeEmpty) query+=QString (" WHERE %1!=''").arg (column);
	return query;
}

QString Database::selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty)
{
	QStringList parts;

	foreach (QString table, tables)
		foreach (QString column, columns)
			parts << selectDistinctColumnQuery (table, column, excludeEmpty);

	return parts.join (" UNION ");
}

QString Database::selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty)
{
	return selectDistinctColumnQuery (tables, QStringList (column), excludeEmpty);
}

QString Database::selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty)
{
	return selectDistinctColumnQuery (QStringList (table), columns, excludeEmpty);
}

bool Database::queryHasResult (QSqlQuery &query)
{
	executeQuery (query);
	return query.size()>0;
}

bool Database::queryHasResult (QString queryString)
{
	QSqlQuery query=executeQuery (queryString);
	return query.size()>0;
}

void Database::updateColumnValues (const QString &tableName, const QString &columnName,
	const QVariant &oldValue, const QVariant &newValue)
{
	QSqlQuery query=prepareQuery (
		QString ("UPDATE %1 SET %2=? WHERE %2=?")
		.arg (tableName, columnName)
	);

	query.addBindValue (newValue);
	query.addBindValue (oldValue);

	executeQuery (query);
}



// *************************
// ** Schema manipulation **
// *************************

void Database::createTable (const QString &name, bool skipIfExists)
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

void Database::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	std::cout << QString ("Creating table %1 like %2%3")
		.arg (name, like, skipIfExists?" if it does not exist":"")
		<< std::endl;

	QString queryString=
		QString ("CREATE TABLE %1 %2 LIKE %3")
		.arg (skipIfExists?"IF NOT EXISTS":"", name, like);

	executeQuery (queryString);
}

void Database::dropTable (const QString &name)
{
	std::cout << QString ("Dropping table %1").arg (name) << std::endl;

	QString queryString=
		QString ("DROP TABLE %1")
		.arg (name);

	executeQuery (queryString);
}

void Database::renameTable (const QString &oldName, const QString &newName)
{
	std::cout << QString ("Renaming table %1 to %2").arg (oldName, newName) << std::endl;

	QString queryString=
		QString ("RENAME TABLE %1 TO %2")
		.arg (oldName, newName);

	executeQuery (queryString);
}

bool Database::tableExists (const QString &name)
{
	// Using addBindValue does not seem to work here
	QString queryString=
		QString ("SHOW TABLES LIKE '%1'")
		.arg (name);

	return queryHasResult (queryString);
}

void Database::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
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

void Database::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	std::cout << QString ("Changing column %1.%2 type to %3")
		.arg (table, name, type) << std::endl;

	QString queryString=
		QString ("ALTER TABLE %1 MODIFY %2 %3 %4")
		.arg (table, name, type, extraSpecification);

	executeQuery (queryString);
}

void Database::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
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

void Database::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	std::cout << QString ("Renaming column %1.%2 to %3").arg (table, oldName, newName) << std::endl;

	QString queryString=
		QString ("ALTER TABLE %1 CHANGE %2 %3 %4 %5")
		.arg (table, oldName, newName, type, extraSpecification);

	executeQuery (queryString);
}

bool Database::columnExists (const QString &table, const QString &name)
{
	// Using addBindValue does not seem to work here
	QString queryString=
		QString ("SHOW COLUMNS FROM %1 LIKE '%2'")
		.arg (table, name);

	return queryHasResult (queryString);
}


// *********
// ** ORM **
// *********

template<class T> QList<T> Database::getObjects (QString condition, QList<QVariant> conditionValues)
{
	QSqlQuery query (db);
	query.setForwardOnly (true);

	QString queryString=QString ("SELECT %1 FROM %2")
		.arg (T::selectColumnList (), T::dbTableName ());

	if (!condition.isEmpty ())
		queryString+=" WHERE "+condition;

	query.prepare (queryString);

	foreach (const QVariant &conditionValue, conditionValues)
		query.addBindValue (conditionValue);

	executeQuery (query);

    return T::createListFromQuery (query);
}

template<class T> int Database::countObjects ()
{
	QString q="SELECT COUNT(*) FROM "+T::dbTableName ();
	QSqlQuery query (q, db);

	query.next ();
    return query.value (0).toInt ();
}

template<class T> bool Database::objectExists (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("SELECT COUNT(*) FROM "+T::dbTableName ()+" WHERE id=?");
	query.addBindValue (id);
	executeQuery (query);

	query.next ();
	return query.value (0).toInt ()>0;
}

template<class T> T Database::getObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("SELECT "+T::selectColumnList ()+" FROM "+T::dbTableName ()+" WHERE ID=?");
	query.addBindValue (id);
	executeQuery (query);

	if (!query.next ()) throw NotFoundException ();

	return T::createFromQuery (query);
}

template<class T> int Database::deleteObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("DELETE FROM "+T::dbTableName ()+" WHERE ID=?");
	query.addBindValue (id);
	executeQuery (query);

	return query.numRowsAffected ();
}

/**
 * The id of the object is ignored and overwritten.
 *
 * @param object
 * @return
 */
template<class T> db_id Database::createObject (T &object)
{
	QSqlQuery query (db);
	query.prepare ("INSERT INTO "+T::dbTableName ()+" "+T::insertValueList ());
	object.bindValues (query);
	executeQuery (query);

	object.id=query.lastInsertId ().toLongLong ();

	return object.id;
}

template<class T> int Database::updateObject (const T &object)
{
	QSqlQuery query (db);
	query.prepare ("UPDATE "+T::dbTableName ()+" SET "+object.updateValueList ()+" WHERE id=?");
	object.bindValues (query);
	query.addBindValue (object.id);

	executeQuery (query);

	return query.numRowsAffected ();
}

// Instantiate the class templates
// Classes have to provide:
//   - ::dbTableName ();
//   - ::QString selectColumnList ();
//   - ::createFromQuery (const QSqlQuery &query);
//   - ::insertValueList ();
//   - ::updateValueList ();
//   - bindValues (QSqlQuery &q) const;
//   - ::createListFromQuery (QSqlQuery &query);

template QList<Person    > Database::getObjects       (QString condition, QList<QVariant> conditionValues);
template QList<Plane     > Database::getObjects       (QString condition, QList<QVariant> conditionValues);
template QList<Flight    > Database::getObjects       (QString condition, QList<QVariant> conditionValues);
template QList<LaunchMethod> Database::getObjects       (QString condition, QList<QVariant> conditionValues);

template int           Database::countObjects<Person    > ();
template int           Database::countObjects<Plane     > ();
template int           Database::countObjects<Flight    > ();
template int           Database::countObjects<LaunchMethod> ();

template bool          Database::objectExists<Person    > (db_id id);
template bool          Database::objectExists<Plane     > (db_id id);
template bool          Database::objectExists<Flight    > (db_id id);
template bool          Database::objectExists<LaunchMethod> (db_id id);

template Person        Database::getObject            (db_id id);
template Plane         Database::getObject            (db_id id);
template Flight        Database::getObject            (db_id id);
template LaunchMethod    Database::getObject            (db_id id);

template int           Database::deleteObject<Person    > (db_id id);
template int           Database::deleteObject<Plane     > (db_id id);
template int           Database::deleteObject<Flight    > (db_id id);
template int           Database::deleteObject<LaunchMethod> (db_id id);

template db_id         Database::createObject         (Person     &object);
template db_id         Database::createObject         (Plane      &object);
template db_id         Database::createObject         (Flight     &object);
template db_id         Database::createObject         (LaunchMethod &object);

template int           Database::updateObject         (const Person     &object);
template int           Database::updateObject         (const Plane      &object);
template int           Database::updateObject         (const Flight     &object);
template int           Database::updateObject         (const LaunchMethod &object);


// *******************
// ** Very specific **
// *******************

QStringList Database::listAirfields ()
{
	return listStrings (selectDistinctColumnQuery (
		Flight::dbTableName (),
		QStringList () << "departure_location" << "landing_location",
		true));
}

QStringList Database::listAccountingNotes ()
{
	return listStrings (selectDistinctColumnQuery (
		Flight::dbTableName (),
		"accounting_notes",
		true));
}

QStringList Database::listClubs ()
{
	return listStrings (selectDistinctColumnQuery (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		"club",
		true));
}

QStringList Database::listPlaneTypes ()
{
	return listStrings (selectDistinctColumnQuery (
		Plane::dbTableName (),
		"type",
		true));
}

QList<Flight> Database::getPreparedFlights ()
{
	// The correct criterion for prepared flights is:
	// !((starts_here and started) or (lands_here and landed))
	// Resolving the flight mode, we get:
	// !( (local and (started or landed)) or (leaving and started) or (coming and landed) )

	QString condition="!( (mode=? AND (departed OR landed)) OR (mode=? AND departed) OR (mode=? AND landed) )";
	QList<QVariant> conditionValues; conditionValues
		<< Flight::modeToDb (Flight::modeLocal  )
		<< Flight::modeToDb (Flight::modeLeaving)
		<< Flight::modeToDb (Flight::modeComing )
		;

	return getObjects<Flight> (condition, conditionValues);
}

QList<Flight> Database::getFlightsDate (QDate date)
{
	// The correct criterion for flights on a given date is:
	// (happened and effective_date=that_date)
	// effective_date has to be calculated from takeoff time, landing time,
	// status and mode, which is compilicated. Thus, we select a superset of
	// the flights of that date and filter out the correct flights afterwards.

	// The superset criterion is:
	// (launch_date=that_date or landing_date=that_date)
	// Since the database stores the datetimes, we compare them agains the
	// first and last datetime of the date.

	QDateTime thisMidnight (date,             QTime (0, 0, 0)); // Start of day
	QDateTime nextMidnight (date.addDays (1), QTime (0, 0, 0)); // Start of next day

	QString condition="(departure_time>=? AND landing_time<?) OR (departure_time>=? AND landing_time<?)";
	QList<QVariant> conditionValues; conditionValues
		<< thisMidnight << nextMidnight
		<< thisMidnight << nextMidnight
		;

	QList<Flight> candidates=getObjects<Flight> (condition, conditionValues);

	// For some of the selected flights, the fact that the takeoff or landing
	// time is on that day may not indicate that the flight actually happened
	// on that day. For example, if a flight is prepared (i. e. not taken off
	// nor landed), or leaving, the times may not be relevant.
	// Thus, we only keep flights which happened and where the effective date
	// is the given date.

	QList<Flight> flights;
	foreach (const Flight &flight, candidates)
		if (flight.happened () && flight.effdatum ()==date)
			flights.append (flight);

	return flights;
}

