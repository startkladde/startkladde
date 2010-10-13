/*
 * Notes:
 *   - the write operations (create, update, delete) are wrapped in a
 *     transaction because when the operation is canceled forcefully (by using
 *     interface#cancelConnection), we cannot determine whether the operation
 *     was performed or not, so the cache may be invalid. Using a transaction
 *     hopefully reduces the "critical" time where this may happen.
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
#include "src/db/Query.h"
#include "src/db/result/Result.h"

// ******************
// ** Construction **
// ******************

Database::Database (Interface &interface):
	interface (interface)
{
}


Database::~Database()
{
}


// ****************
// ** Connection **
// ****************

void Database::cancelConnection ()
{
	interface.cancelConnection ();
}

// *********
// ** ORM **
// *********

template<class T> QList<T> Database::getObjects (const Query &condition)
{
	Query query=Query::select (T::dbTableName (), T::selectColumnList ())
		.condition (condition);

	return T::createListFromResult (*interface.executeQueryResult (query));
}

template<class T> int Database::countObjects (const Query &condition)
{
	Query query=Query::count (T::dbTableName ()).condition (condition);
	return interface.countQuery (query);
}

template<class T> bool Database::objectExists (const Query &query)
{
	return countObjects<T> (query)>0;
}

template<class T> bool Database::objectExists (dbId id)
{
	return objectExists<T> (Query ("id=?").bind (id));
}

template<class T> T Database::getObject (dbId id)
{
	Query query=Query::select (T::dbTableName (), T::selectColumnList ())
		.condition (Query ("id=?").bind (id));

	QSharedPointer<Result> result=interface.executeQueryResult (query);

	if (!result->next ()) throw NotFoundException ();

	return T::createFromResult (*result);
}

template<class T> bool Database::deleteObject (dbId id)
{
	Query query=Query ("DELETE FROM %1 WHERE ID=?")
		.arg (T::dbTableName ()).bind (id);

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	emit dbEvent (DbEvent::deleted<T> (id));

	return result->numRowsAffected ()>0;
}

/**
 * The id of the object is ignored and overwritten.
 *
 * @param object
 * @return
 */
template<class T> dbId Database::createObject (T &object)
{
	Query query=Query ("INSERT INTO %1 (%2) values (%3)")
		.arg (T::dbTableName (), T::insertColumnList (), T::insertPlaceholderList ());
	object.bindValues (query);

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	object.setId (result->lastInsertId ().toLongLong ());

	if (idValid (object.getId ()))
		emit dbEvent (DbEvent::added (object));

	return object.getId ();
}

template<class T> void Database::createObjects (QList<T> &objects, OperationMonitorInterface monitor)
{
	int num=objects.size ();
	monitor.progress (0, num);

	for (int i=0; i<num; ++i)
	{
		createObject (objects[i]);
		monitor.progress (i+1, num);
	}
}

template<class T> bool Database::updateObject (const T &object)
{
	// Use REPLACE INTO instead of UPDATE so in case the object does not exist
	// (e. g. because of an inconsistent cache), it will be created.
	Query query=Query ("REPLACE INTO %1 (id,%2) values (?,%3)")
		.arg (T::dbTableName (), T::insertColumnList (), T::insertPlaceholderList ());
	query.bind (object.getId ());
	object.bindValues (query);

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	emit dbEvent (DbEvent::changed (object));

	return result->numRowsAffected ()>0;
}

template<class T> QList<T> Database::getObjects ()
{
	return getObjects<T> (Query ());
}

template<class T> int Database::countObjects ()
{
	return countObjects<T> (Query ());
}


// *******************
// ** Very specific **
// *******************

QStringList Database::listLocations ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		Flight::dbTableName (),
		QStringList () << "departure_location" << "landing_location" << "towflight_landing_location",
		true));
}

QStringList Database::listAccountingNotes ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		Flight::dbTableName (),
		"accounting_notes",
		true));
}

QStringList Database::listClubs ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		"club",
		true));
}

QStringList Database::listPlaneTypes ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		Plane::dbTableName (),
		"type",
		true));
}


QList<Flight> Database::getPreparedFlights ()
{
	// The correct criterion for prepared flights is:
	// !(happened)
	// also known as
	// !((departs_here and departed) or (lands_here and landed))
	//
	// Resolving the flight mode, we get:
	// !( (local and (departed or landed)) or (leaving and departed) or (coming and landed) )
	//
	// Applying de Morgan (the MySQL query optimizer has a better chance of
	// using an index with AND clauses):
	// !( (local and !(!departed AND !landed)) or (leaving and departed) or (coming and landed) )
	//
	// Applying de Morgan to the outer clause (may not be necessary):
	// !(local and !(!departed AND !landed)) and !(leaving and departed) and !(coming and landed)
	//
	// Note that we test for =0 or !=0 explicitly rather than evaluating the
	// values as booleans (i. e. 'where departed=0' instead of 'where
	// departed') because evaluating as booleans prevents using the index

	// TODO to Flight
	// TODO multi-bind
	Query condition ("!(mode=? AND !(departed=0 AND landed=0)) AND !(mode=? AND departed!=0) AND !(mode=? AND landed!=0)");
	condition.bind (Flight::modeToDb (Flight::modeLocal  ));
	condition.bind (Flight::modeToDb (Flight::modeLeaving));
	condition.bind (Flight::modeToDb (Flight::modeComing ));

	return getObjects<Flight> (condition);
}

QList<Flight> Database::getFlightsDate (QDate date)
{
	// The correct criterion for flights on a given date is:
	// (happened and effective_date=that_date)
	// effective_date has to be calculated from departure time, landing time,
	// status and mode, which is complicated. Thus, we select a superset of
	// the flights of that date and filter out the correct flights afterwards.

	// The superset criterion is:
	// (departure_date=that_date or landing_date=that_date)
	// Since the database stores the datetimes, we compare them agains the
	// first and last datetime of the date.

	QDateTime thisMidnight (date,             QTime (0, 0, 0)); // Start of day
	QDateTime nextMidnight (date.addDays (1), QTime (0, 0, 0)); // Start of next day

	// TODO to Flight (but need after-filter)
	// TODO multi-bind
	Query condition ("(departure_time>=? AND departure_time<?) OR (landing_time>=? AND landing_time<?)");
	condition.bind (thisMidnight); condition.bind (nextMidnight);
	condition.bind (thisMidnight); condition.bind (nextMidnight);

	QList<Flight> candidates=getObjects<Flight> (condition);

	// For some of the selected flights, the fact that the departure or landing
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

template<class T> bool Database::objectUsed (dbId id)
{
	(void)id;
	// Return true for safety; specialize for specific classes
	return true;
}

template<> bool Database::objectUsed<Person> (dbId id)
{
	// ATTENTION: make sure that DbManager::mergePeople correspondents to this
	// method

	// A person may be referenced by a flight
	if (objectExists<Flight> (Flight::referencesPersonCondition (id))) return true;

	// A person may be referenced by a user (although we don't have a user
	// model here, only in sk_web)
	if (interface.countQuery (
		Query::count ("users", Query ("person_id=?").bind (id))
	)) return true;

	return false;
}

template<> bool Database::objectUsed<Plane> (dbId id)
{
	// Launch methods reference planes by registration, which we don't have
	// to check.

	// A plane may be reference by a flight
	return objectExists<Flight> (Flight::referencesPlaneCondition (id));
}

template<> bool Database::objectUsed<LaunchMethod> (dbId id)
{
	// A launch method may be reference by a flight
	return objectExists<Flight> (Flight::referencesLaunchMethodCondition (id));
}

template<> bool Database::objectUsed<Flight> (dbId id)
{
	(void)id;

	// Flights are never used
	return false;
}


// **********
// ** Misc **
// **********

void Database::emitDbEvent (DbEvent event)
{
	// Hack for merging people, invoked by our friend DbManager::mergePeople
	emit dbEvent (event);
}

// ***************************
// ** Method instantiations **
// ***************************

// Instantiate the method templates
// Classes have to provide:
//   - ::dbTableName ();
//   - ::QString selectColumnList (); // TODO return queries directly?
//   - ::createFromQuery (const Result &result); // TODO change to create
//   - ::insertColumnList ();
//   - ::insertPlaceholderList ();
//   - bindValues (QSqlQuery &q) const;
//   - ::createListFromQuery (Result &result); // TODO change to createList

#define INSTANTIATE_TEMPLATES(T) \
	template QList<T> Database::getObjects      (const Query &condition); \
	template int      Database::countObjects<T> (const Query &condition); \
	template bool     Database::objectExists<T> (dbId id); \
	template T        Database::getObject       (dbId id); \
	template bool     Database::deleteObject<T> (dbId id); \
	template dbId     Database::createObject    (T &object); \
	template void     Database::createObjects   (QList<T> &objects, OperationMonitorInterface monitor); \
	template bool     Database::updateObject    (const T &object); \
	template QList<T> Database::getObjects  <T> (); \
	template int      Database::countObjects<T> (); \
	template bool     Database::objectUsed<T>   (dbId id);

	// Empty line

INSTANTIATE_TEMPLATES (Person      )
INSTANTIATE_TEMPLATES (Plane       )
INSTANTIATE_TEMPLATES (Flight      )
INSTANTIATE_TEMPLATES (LaunchMethod)

#undef INSTANTIATE_TEMPLATES
