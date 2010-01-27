/*
 * Short term plan:
 *   - add to Database:
 *     - flights prepared and flights by date
 *   - change DataStorage to use this class
 *   - remove all editable
 *   - remove old database
 *   - remove old db_proxy and admin_functions (check no longer needed)
 *   - reenable database checking (?) (must show what to change)
 *   - add creating the database
 *   - get rid of EntityType
 *   - Clean up constructors: create empty (id 0), create with id
 *   - Standardize enum handling: store the database value internally (or use the
 *     numeric value in the database?); and have an "unknown" type (instead of "none")
 *   - do we really want to read the clubs etc. from the database in DataStorage?
 *   - add "object used" to Database
 *   - add ping to database
 *
 * Improvements:
 *   - split this class into parts (generic, orm, specific)
 *   - move (static) methods like selectDistinctColumnQuery to QueryGenerator (but see below)
 *   - make a class Query, a la Query ("select * from foo where id=?") << 42
 *   - allow specifying an "exclude" value to selectDistinctColumnQuery (requires
 *     Query class)
 *   - move selectDistinctColumnQuery to Query
 *   - move specialized queries generation to model classes (e. g. flight prepared)
 *
 * Medium term plan:
 *   - add some abstraction to the query list generation
 *   - add a version number
 *   - add migrations
 *     - one possibility: with create 0, up/down, and create current
 *     - another: autogenerate current from 0 and ups
 *     - look at rails (or other framework supporting migrations)
 *
 * Long term plan:
 *   - use a memory SQLite for local storage (datastorage functionality)
 *   - merge data storage and database methods, so we can use a (local)
 *     database directly (w/o local cache)
 *       - we'd probably have abstract DataStorage and inherited Database and
 *         CachingDataStorage which uses ad Database for access and one (in
 *         memory) for local caching
 *       - question: can we afford using a query on a memory database e. g.
 *         for list of first names rather than maintaining the list explicitly?
 *         should be fast enough (does sqlite memory table have index?)
 *   - local disk caching
 */

#include "Database.h"

#include <iostream>
#include <cassert>

#include <QDateTime>

#include "src/config/Options.h"
#include "src/text.h"

// TODO: check if the slow part is the loop; if yes, add ProgressMonitor

// TODO: error when query fails

/*
 * Here's an idea for specifying conditions, or parts thereof:
 * Condition ("foo=? and bar=?") << 42 << "baz";
 */

Database::Database ()
{
    db=QSqlDatabase::addDatabase ("QMYSQL");
}

bool Database::open (QString server, int port, QString username, QString password, QString database)
{
    db.setHostName (server);
    db.setUserName (username);
    db.setPassword (password);
    db.setPort (port);
    db.setDatabaseName (database);

    bool result=db.open ();
	if (!result) return false;

//    QSqlQuery query (db);
//    query.prepare ("show variables like 'char%'");
//    query.exec ();
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
	q.exec ();
	return listStrings (q);
}

QString Database::selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty)
{
	// "select distinct column from table"
	QString query=QString ("select distinct %1 from %2").arg (column, table);

	// ..." where column!=''"
	if (excludeEmpty) query+=QString (" where %1!=''").arg (column);
	return query;
}

QString Database::selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty)
{
	QStringList parts;

	foreach (QString table, tables)
		foreach (QString column, columns)
			parts << selectDistinctColumnQuery (table, column, excludeEmpty);

	return parts.join (" union ");
}

QString Database::selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty)
{
	return selectDistinctColumnQuery (tables, QStringList (column), excludeEmpty);
}

QString Database::selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty)
{
	return selectDistinctColumnQuery (QStringList (table), columns, excludeEmpty);
}


// *********
// ** ORM **
// *********

template<class T> QList<T> Database::getObjects (QString condition, QList<QVariant> conditionValues)
{
	QSqlQuery query (db);
	query.setForwardOnly (true);

	QString queryString=QString ("select %1 from %2")
		.arg (T::selectColumnList (), T::dbTableName ());

	if (!condition.isEmpty ())
		queryString+=" where "+condition;

	query.prepare (queryString);

	foreach (const QVariant &conditionValue, conditionValues)
		query.addBindValue (conditionValue);

	query.exec ();

    return T::createListFromQuery (query);
}

template<class T> int Database::countObjects ()
{
	QString q="select count(*) from "+T::dbTableName ();
	QSqlQuery query (q, db);

	query.next ();
    return query.value (0).toInt ();
}

template<class T> bool Database::objectExists (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("select count(*) from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	query.next ();
	return query.value (0).toInt ()>0;
}

template<class T> T Database::getObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("select "+T::selectColumnList ()+" from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	if (!query.next ()) throw NotFoundException ();

	return T::createFromQuery (query);
}

template<class T> int Database::deleteObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("delete from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	return query.numRowsAffected ();
}

template<class T> db_id Database::createObject (T &object)
{
	QSqlQuery query (db);
	query.prepare ("insert into "+T::dbTableName ()+" "+T::insertValueList ());
	object.bindValues (query);
	query.exec ();

	object.id=query.lastInsertId ().toLongLong ();

	return object.id;
}

template<class T> int Database::updateObject (const T &object)
{
	QSqlQuery query (db);
	query.prepare ("update "+T::dbTableName ()+" set "+object.updateValueList ()+" where id=?");
	object.bindValues (query);
	query.addBindValue (object.id);

	query.exec ();

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

#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/LaunchType.h"

template QList<Person> Database::getObjects           (QString condition, QList<QVariant> conditionValues);
template QList<Plane > Database::getObjects           (QString condition, QList<QVariant> conditionValues);
template QList<Flight> Database::getObjects           (QString condition, QList<QVariant> conditionValues);

template int           Database::countObjects<Person> ();
template int           Database::countObjects<Plane > ();
template int           Database::countObjects<Flight> ();

template bool          Database::objectExists<Person> (db_id id);
template bool          Database::objectExists<Plane > (db_id id);
template bool          Database::objectExists<Flight> (db_id id);

template Person        Database::getObject            (db_id id);
template Plane         Database::getObject            (db_id id);
template Flight        Database::getObject            (db_id id);

template int           Database::deleteObject<Person> (db_id id);
template int           Database::deleteObject<Plane > (db_id id);
template int           Database::deleteObject<Flight> (db_id id);

template db_id         Database::createObject         (Person &object);
template db_id         Database::createObject         (Plane  &object);
template db_id         Database::createObject         (Flight &object);

template int           Database::updateObject         (const Person &object);
template int           Database::updateObject         (const Plane  &object);
template int           Database::updateObject         (const Flight &object);

// Legacy launch type
template<> QList<LaunchType> Database::getObjects (QString condition, QList<QVariant> conditionValues)
{
	assert (condition.isEmpty ());
	(void)conditionValues;
	return launchTypes.values ();
}

template<> int Database::countObjects<LaunchType> ()
{
	return launchTypes.size ();
}

template<> bool Database::objectExists<LaunchType> (db_id id)
{
	return launchTypes.contains (id);
}

template<> LaunchType Database::getObject (db_id id)
{
	if (launchTypes.contains (id)) throw NotFoundException ();
	return launchTypes[id];
}

template<> int Database::deleteObject<LaunchType> (db_id id)
{
	(void)id;
	assert (!"Thou shalt not try to delete a launch type");
	return 0;
}

template<> db_id Database::createObject (LaunchType &object)
{
	(void)object;
	assert (!"Thou shalt not try to create a launch type");
	return invalid_id;
}

template<> int Database::updateObject (const LaunchType &object)
{
	(void)object;
	assert (!"Thou shalt not try to update a launch type");
	return 0;
}

void Database::addLaunchType (const LaunchType &launchType)
{
	launchTypes.insert (launchType.get_id (), launchType);
}


// *******************
// ** Very specific **
// *******************

QStringList Database::listAirfields ()
{
	QString query="select startort from %1 where startort!='' union select zielort from %1 where zielort!=''";
	return listStrings (query.arg (Flight::dbTableName ()));

	return listStrings (selectDistinctColumnQuery (
		Flight::dbTableName (),
		QStringList () << "startort" << "zielort"
		));
}

QStringList Database::listAccountingNotes ()
{
	return listStrings (selectDistinctColumnQuery (
		Flight::dbTableName (),
		"abrechnungshinweis",
		true));
}

QStringList Database::listClubs ()
{
	return listStrings (selectDistinctColumnQuery (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		"verein",
		true));
}

QStringList Database::listPlaneTypes ()
{
	return listStrings (selectDistinctColumnQuery (
		Plane::dbTableName (),
		"typ",
		true));
}

QList<Flight> Database::getPreparedFlights ()
{
	// TODO  Correct criterion is:
	// happened=(starts here and started) or (lands here and lande)
	//   -> maybe resolve with def's for starts/lands here
	// prepared=!happened
	return getObjects<Flight> ("status=?", QList<QVariant> () << 0);
}

QList<Flight> Database::getFlightsDate (QDate date)
{
//	QDateTime firstDate (date, QTime (0, 0, 0)); // Start of day
//	QDateTime lastDate (date.addDays (1), QTime (0, 0, 0)); // Start of next day

	// TODO make it right
	// The decision can be partly implemented in the client.
	// Old:
	// on date = happened and effective date is that date
	// effective date: (starts here and not (only landed))
	//   startzeit
	// else
	//   landezeit


	QString condition="true";
	QList<QVariant> conditionValues;
	QList<Flight> flights=getObjects<Flight> (condition, conditionValues);

	return flights;
}
