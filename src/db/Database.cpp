/*
 * Next:
 *   - integrate schema loading/database migration into GUI
 *
 * Short term plan:
 *   - Test the migrations:
 *     - make sure the old version and current sk_web work with "initial"
 *     - make sure we can migrate from both an empty and the legacy database
 *   - Fixtures
 *     - SQL dump or data file (CSV/YAML)?
 *     - C++ or Ruby?
 *   - Standardize enum handling: store the database value internally and have
 *     an "unknown" type (instead of "none")
 *     - this should also allow preserving unknown types in the database
 *   - add "object used" to Database
 *   - add ping to Database
 *   - generate string lists from other data instead of explicit query (but still
 *     cache explicitly) note that we still have to query for accounting notes and
 *     locations because we don't get a complete flight list
 *   - fix error reporting (db.lastError (), e. g. delete objects)
 *   - make sure "", 0 and NULL are read correctly
 *
 * Improvements:
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
 *         locations are more interesting, b/c there are many flights
 *   - local disk caching
 */

#include "Database.h"

#include <iostream>
#include <cassert>

#include <QDateTime>

#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/util/qString.h"


/*
 * Here's an idea for specifying conditions, or parts thereof:
 * Condition ("foo=? and bar=?") << 42 << "baz";
 */



// ******************
// ** Construction **
// ******************

Database::Database ():
	databaseInterface ()
{
}


Database::~Database()
{
}








// *********
// ** ORM **
// *********

template<class T> QList<T> Database::getObjects (QString condition, QList<QVariant> conditionValues)
{
	QString queryString=QString ("SELECT %1 FROM %2")
		.arg (T::selectColumnList (), T::dbTableName ());

	if (!condition.isEmpty ())
		queryString+=" WHERE "+condition;

	QSqlQuery query=databaseInterface.prepareQuery (queryString, true);
	foreach (const QVariant &conditionValue, conditionValues)
		query.addBindValue (conditionValue);

	databaseInterface.executeQuery (query);

    return T::createListFromQuery (query);
}

template<class T> int Database::countObjects ()
{
	QString queryString="SELECT COUNT(*) FROM "+T::dbTableName ();

	QSqlQuery query=databaseInterface.executeQuery (queryString);

	query.next ();
    return query.value (0).toInt ();
}

template<class T> bool Database::objectExists (dbId id)
{
	QString queryString=QString ("SELECT COUNT(*) FROM %1 WHERE id=?")
		.arg (T::dbTableName ());

	QSqlQuery query=databaseInterface.prepareQuery (queryString);
	query.addBindValue (id);
	databaseInterface.executeQuery (query);

	query.next ();
	return query.value (0).toInt ()>0;
}

template<class T> T Database::getObject (dbId id)
{
	QString queryString=QString ("SELECT %1 FROM %2 WHERE ID=?")
		.arg (T::selectColumnList (), T::dbTableName ());

	QSqlQuery query=databaseInterface.prepareQuery (queryString);
	query.addBindValue (id);
	databaseInterface.executeQuery (query);

	if (!query.next ()) throw NotFoundException ();

	return T::createFromQuery (query);
}

template<class T> int Database::deleteObject (dbId id)
{
	QString queryString=QString ("DELETE FROM %1 WHERE ID=?")
		.arg (T::dbTableName ());

	QSqlQuery query=databaseInterface.prepareQuery (queryString);
	query.addBindValue (id);
	databaseInterface.executeQuery (query);

	return query.numRowsAffected ();
}

/**
 * The id of the object is ignored and overwritten.
 *
 * @param object
 * @return
 */
template<class T> dbId Database::createObject (T &object)
{
	QString queryString=QString ("INSERT INTO %1 %2")
		.arg (T::dbTableName (), T::insertValueList ());

	QSqlQuery query=databaseInterface.prepareQuery (queryString);
	object.bindValues (query);
	databaseInterface.executeQuery (query);

	object.id=query.lastInsertId ().toLongLong ();

	return object.id;
}

template<class T> int Database::updateObject (const T &object)
{
	QString queryString=QString ("UPDATE %1 SET %2 WHERE id=?")
		.arg (T::dbTableName (), object.updateValueList ());

	QSqlQuery query=databaseInterface.prepareQuery (queryString);
	object.bindValues (query);
	query.addBindValue (object.id);

	databaseInterface.executeQuery (query);

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

#define INSTANTIATE_TEMPLATES(Class) \
	template QList<Class> Database::getObjects (QString condition, QList<QVariant> conditionValues); \
	template int          Database::countObjects<Class> (); \
	template bool         Database::objectExists<Class> (dbId id); \
	template Class        Database::getObject           (dbId id); \
	template int          Database::deleteObject<Class> (dbId id); \
	template dbId         Database::createObject        (Class &object); \
	template int          Database::updateObject        (const Class &object); \
	// Empty line

INSTANTIATE_TEMPLATES (Person      )
INSTANTIATE_TEMPLATES (Plane       )
INSTANTIATE_TEMPLATES (Flight      )
INSTANTIATE_TEMPLATES (LaunchMethod)

#undef INSTANTIATE_TEMPLATES


// *******************
// ** Very specific **
// *******************

QStringList Database::listLocations ()
{
	return databaseInterface.listStrings (databaseInterface.selectDistinctColumnQuery (
		Flight::dbTableName (),
		QStringList () << "departure_location" << "landing_location",
		true));
}

QStringList Database::listAccountingNotes ()
{
	return databaseInterface.listStrings (databaseInterface.selectDistinctColumnQuery (
		Flight::dbTableName (),
		"accounting_notes",
		true));
}

QStringList Database::listClubs ()
{
	return databaseInterface.listStrings (databaseInterface.selectDistinctColumnQuery (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		"club",
		true));
}

QStringList Database::listPlaneTypes ()
{
	return databaseInterface.listStrings (databaseInterface.selectDistinctColumnQuery (
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

