/*
 * Next:
 *   - integrate schema loading/database migration into GUI
 *
 * Short term plan:
 *   - Standardize enum handling: store the database value internally and have
 *     an "unknown" type (instead of "none")
 *     - this should also allow preserving unknown types in the database
 *   - add "object used?" to Database (and use in ObjectListWindow)
 *   - add ping to Interface
 *   - timeout: only when no data is transfered
 *
 * Tests:
 *   - Test the migrations:
 *     - make sure the old version and current sk_web work with "initial"
 *     - make sure we can migrate from both an empty and the legacy database
 *   - Fixtures
 *     - SQL dump or data file (CSV/YAML)?
 *     - C++ or Ruby?
 *   - Port open, but connection not accepted
 *   - make sure "", 0 and NULL are read correctly
 *   - make sure (using a dump of an old db) that the migration path is OK
 *
 * Improvements:
 *   - Add a ResultConsumer as alternative to passing a result (especially for
 *     CopiedResult)
 *   - allow specifying an "exclude" value to selectDistinctColumns (what for?)
 *   - move specialized queries generation to model classes (e. g. flight prepared)
 *     (but need after-filter)
 *   - Flight should be an entity
 *   - Allow tables without id column (e. g. habtm join tables), but tables
 *     must have at least one column
 *   - Remove code multiplication in Models (SQL interface)
 *   - change flight mode (and towflight mode) column type
 *   - Add a Database::saveObject that creates or updates the object, depending
 *     on whether it has a valid id
 *   - Allow a table name prefix (useful for testing)
 *
 * Medium term plan:
 *   - rename planes.competition_callsign to planes.callsign
 *   - add some abstraction to the query list generation
 *   - add support for sqlite
 *   - add indexes
 *   - add foreign key constraints
 *   - database checks (?)
 *     - show what is wrong/will be fixed
 *
 * Long term plan:
 *   - use a memory SQLite for local storage (datastorage functionality)
 *   - merge data storage and database methods, so we can use a (local)
 *     database directly (w/o local cache)
 *       - we'd probably have abstract DataStorage and inherited Database and
 *         CachingDataStorage which uses a Database for access and one (in
 *         memory) for local caching
 *       - question: can we afford using a query on a memory database e. g.
 *         for list of first names rather than maintaining the list explicitly?
 *         should be fast enough (does sqlite memory table have index?)
 *         locations are more interesting, b/c there are many flights
 *   - local disk caching
 */

/*
 * Old DataStorage sez:
 *
 * On multithreaded database access:
 *   - Some operations may take some time, especially over a slow network
 *     connection. Examples:
 *       - refresh cache, fetch flights for a given date (long operation even
 *         with local storage)
 *       - write to the database (create, update, delete)
 *   - Some operations cannot be interrupted, for example a MySQL call waiting
 *     for a timeout due to a non-working network connection
 *   - We want to be able to cancel such operations. For this, we must have a
 *     responsive GUI, so the GUI thread must be running
 *   - Typically, the function initiating a long operation will not continue
 *     (which also means that it will not return) until after the task is
 *     completed (or failed or aborted), as the next action may depend on the
 *     result of the task
 *
 * Implementation of multithreaded database access:
 *   - operation are performed by a Task
 *   - the DataStorage has a WorkerThread that performs Tasks sequentially on
 *     a background thread
 *   - the database access functionality is in DataStorage; the tasks call the
 *     corresponding method of DataStorage, passing a pointer the Task as an
 *     OperationMonitor
 *   - a dialog for monitoring and canceling Tasks is provided by
 *     TaskProgressDialog
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

namespace Db
{
	// ******************
	// ** Construction **
	// ******************

	Database::Database (Interface::Interface &interface):
		interface (interface)
	{
	}


	Database::~Database()
	{
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

		QSharedPointer<Result::Result> result=interface.executeQueryResult (query);

		if (!result->next ()) throw NotFoundException ();

		return T::createFromResult (*result);
	}

	template<class T> bool Database::deleteObject (dbId id)
	{
		Query query=Query ("DELETE FROM %1 WHERE ID=?")
			.arg (T::dbTableName ()).bind (id);

		QSharedPointer<Result::Result> result=interface.executeQueryResult (query);

		emit dbEvent (Event::Event (Event::Event::typeDelete, Event::Event::getTable<T> (), id));

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
		Query query=Query ("INSERT INTO %1 %2").arg (T::dbTableName (), T::insertValueList ());
		object.bindValues (query);

		QSharedPointer<Result::Result> result=interface.executeQueryResult (query);

		object.id=result->lastInsertId ().toLongLong ();

		if (idValid (object.id))
			emit dbEvent (Event::Event (Event::Event::typeAdd, Event::Event::getTable<T> (), object.id));

		return object.id;
	}

	template<class T> bool Database::updateObject (const T &object)
	{
		Query query=Query ("UPDATE %1 SET %2 WHERE id=?")
			.arg (T::dbTableName (), object.updateValueList ());

		object.bindValues (query);
		query.bind (object.id); // After the object values!

		QSharedPointer<Result::Result> result=interface.executeQueryResult (query);

		emit dbEvent (Event::Event (Event::Event::typeChange, Event::Event::getTable<T> (), object.getId ()));

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
			QStringList () << "departure_location" << "landing_location",
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
		// !((starts_here and started) or (lands_here and landed))
		// Resolving the flight mode, we get:
		// !( (local and (started or landed)) or (leaving and started) or (coming and landed) )

		// TODO to Flight
		// TODO multi-bind
		Query condition ("!( (mode=? AND (departed OR landed)) OR (mode=? AND departed) OR (mode=? AND landed) )");
		condition.bind (Flight::modeToDb (Flight::modeLocal  ));
		condition.bind (Flight::modeToDb (Flight::modeLeaving));
		condition.bind (Flight::modeToDb (Flight::modeComing ));

		return getObjects<Flight> (condition);
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

		// TODO to Flight
		// TODO multi-bind
		Query condition ("(departure_time>=? AND landing_time<?) OR (departure_time>=? AND landing_time<?)");
		condition.bind (thisMidnight); condition.bind (nextMidnight);
		condition.bind (thisMidnight); condition.bind (nextMidnight);

		QList<Flight> candidates=getObjects<Flight> (condition);

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

	template<class T> bool Database::objectUsed (dbId id)
	{
		(void)id;
		// Return true for safety; specialize for specific classes
		return true;
	}

	template<> bool Database::objectUsed<Person> (dbId id)
	{
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

	// ***************************
	// ** Method instantiations **
	// ***************************

	// Instantiate the method templates
	// Classes have to provide:
	//   - ::dbTableName ();
	//   - ::QString selectColumnList (); // TODO return queries directly?
	//   - ::createFromQuery (const Result &result); // TODO change to create
	//   - ::insertValueList ();
	//   - ::updateValueList ();
	//   - bindValues (QSqlQuery &q) const;
	//   - ::createListFromQuery (Result &result); // TODO change to createList

#	define INSTANTIATE_TEMPLATES(T) \
		template QList<T> Database::getObjects      (const Query &condition); \
		template int      Database::countObjects<T> (const Query &condition); \
		template bool     Database::objectExists<T> (dbId id); \
		template T        Database::getObject       (dbId id); \
		template bool     Database::deleteObject<T> (dbId id); \
		template dbId     Database::createObject    (T &object); \
		template bool     Database::updateObject    (const T &object); \
		template QList<T> Database::getObjects  <T> (); \
		template int      Database::countObjects<T> (); \
		template bool     Database::objectUsed<T>   (dbId id);

		// Empty line

	INSTANTIATE_TEMPLATES (Person      )
	INSTANTIATE_TEMPLATES (Plane       )
	INSTANTIATE_TEMPLATES (Flight      )
	INSTANTIATE_TEMPLATES (LaunchMethod)

#	undef INSTANTIATE_TEMPLATES

}
