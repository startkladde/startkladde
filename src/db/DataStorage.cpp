#include "DataStorage.h"

#include <iostream>

#include <QSet>
#include <QThread>
#include <QProgressDialog>
#include <QApplication>
#include <QMutableListIterator>

#include "src/concurrent/threadUtil.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/DefaultQThread.h"
#include "src/concurrent/monitor/SimpleOperationMonitor.h"

using namespace std;

/*
 * Thread safety note:
 *   - this class is completely thread safe
 *   - all accesses to internal data, even single values, must be protected by
 *     dataMutex
 *   - all calls to methods of db must be protected by databaseMutex
 */

/*
 * Database monitor:
 *   - display state OK/not OK in main window (through signal)
 *   - start program:
 *     - display main window always, with status "Connecting..."
 *       => sensible behavior when database not OK
 *     - check if DB ok, if not => try to make OK
 */

/*
 * Multithreading TODO:
 * TODO: Potential problem: refresh, cancel, refresh. First refresh still
 *       running. Can the second one be started? May the first one hang even
 *       though the database comes back online?
 * TODO: when to delete tasks that take some time to cancel
 * TODO: add Task* parameter to signals (but connection to dialog?)
 * TODO: fortune example:
 *   - thread destructor: quit = true; cond.wakeOne(); wait();
 *   - invoking: if (!isRunning()) start(); else cond.wakeOne();
 * TODO: test case: port open, but connection not accepted
 * TODO: display error, db->get_last_error
 *
 * TODO: handle unusable database
 *
 * TODO: the reconnecting (1 second delay) and associated error handling is
 * done in several places
 * TODO: all database actions should retry until success or canceled.
 * TODO: on timeout in an operation (other then the idle ping), the state
 * should be updated immediately - same for a succeeded query
 */

/*
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

/*
 * On local storage:
 *   - Not yet implemented
 *   - Writes to the database are not long operations with a local storage
 */


/*
 * TODO:
 *   - prepared flights
 *   - this must probably have some concept of an "active date", so we can also
 *     edit flights and create logs for past days.
 *   - use QSet where appropriate, but sort case insensitively
 *   - notify the main window about changes (old db_change mechanism?)
 *   - log an error if an invalid ID is passed to the get by ID functions
 *   - addObject error message when getX (maybe also getNewX) is called with an
 *     invalid ID, this should be checked by the caller
 *   - don't delete an entity that is still in use; potentially addObject a force
 *     flag
 *   - For retrieving flights of other days, we need a way to say "Get this",
 *     then wait until it finished, and have a way of aborting it or signalling
 *     error.
 *   - club list should not include ""/whitespace only; Orte dito
 */


// ******************
// ** Construction **
// ******************

DataStorage::DataStorage (OldDatabase &db):
	db (db), //workerThread ("DataStorage worker"),
	currentState (stateOffline), worker (*this, 1000)
{
	QObject::connect (&db, SIGNAL (executing_query (QString)), this, SIGNAL (executingQuery (QString)));
	worker.start ();
}

DataStorage::~DataStorage ()
{
	// This causes the program to hang on quit when the server is unresponsive
//	worker.stop (true);

	// This is not nice, but it will have to do for now.
	// TODO this should not be in the destructor, a terminate method would be
	// better, so we can terminate all threads before waiting
	worker.setTerminationEnabled (true);
	worker.terminate ();
	std::cout << "Wating for data storage worker to terminate..."; std::cout.flush ();
	if (worker.wait (1000))
		std::cout << "done" << std::endl;
	else
		std::cout << "timeout" << std::endl;
}

// **********************
// ** Database reading **
// **********************

#define copyList(T, source, target) do \
{ \
	target.clear (); \
	foreach (T *it, source) \
		target.append (*it); \
} while (0)

#define copyListLocked(T, source, target) do \
{ \
	QMutexLocker lock (&dataMutex); \
	copyList (T, source, target); \
	lock.unlock (); \
} while (0)


int DataStorage::refreshPlanes ()
{
	QList<Plane *> planeList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_planes_all (planeList);
	dbLock.unlock ();

	if (db_ok==result)
		copyListLocked (Plane, planeList, planes);

	deletePointerList (Plane, planeList);

	return result;
}

int DataStorage::refreshPeople ()
{
	QList<Person *> personList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_persons_all (personList);
	dbLock.unlock ();

	if (db_ok==result)
		copyListLocked (Person, personList, people);

	deletePointerList (Person, personList);

	return result;
}

int DataStorage::refreshLaunchTypes ()
{
	QList<LaunchType *> launchTypeList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_startarten_all (launchTypeList);
	dbLock.unlock ();

	if (db_ok==result)
		copyListLocked (LaunchType, launchTypeList, launchTypes);

	deletePointerList (LaunchType, launchTypeList);

	return result;
}

int DataStorage::refreshFlights (QDate date, EntityList<Flight> &listTarget, QDate *dateTarget)
{
	QList<Flight *> flightList;

	QMutexLocker dbLock (&databaseMutex);
	int result;
	if (date.isNull ())
		result=db.list_flights_prepared (flightList);
	else
		result=db.list_flights_date (flightList, &date);
	dbLock.unlock ();

	if (db_ok==result)
	{
		if (dateTarget) *dateTarget=QDate ();
		QMutexLocker lock (&dataMutex);
		copyList (Flight, flightList, listTarget);
		if (dateTarget) *dateTarget=date;
		lock.unlock ();
	}

	deletePointerList (Flight, flightList);

	return result;
}

/**
 * Frontend to refreshFlights
 */
int DataStorage::refreshFlightsToday ()
{
	return refreshFlights (QDate::currentDate (), flightsToday, &todayDate);
}

/**
 * Frontend to refreshFlights
 */
int DataStorage::refreshPreparedFlights ()
{
	return refreshFlights (QDate (), preparedFlights, NULL);
}

int DataStorage::refreshAirfields ()
{
	QStringList airfieldList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_airfields (airfieldList);
	dbLock.unlock ();

	if (db_ok==result)
	{
		QMutexLocker lock (&dataMutex);
		airfields=airfieldList;
		lock.unlock ();
	}

	return result;
}

int DataStorage::refreshAccountingNotes ()
{
	QStringList accountingNoteList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_accounting_note (accountingNoteList);
	dbLock.unlock ();

	if (db_ok==result)
	{
		QMutexLocker lock (&dataMutex);
		accountingNotes=accountingNoteList;
		lock.unlock ();
	}

	return result;
}

int DataStorage::refreshClubs ()
{
	QStringList clubList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_clubs (clubList);
	dbLock.unlock ();

	// TODO generate from planes/people?
	if (db_ok==result)
	{
		QMutexLocker lock (&dataMutex);
		clubs=clubList;
		lock.unlock ();
	}

	return result;
}

int DataStorage::refreshPlaneTypes ()
{
	QStringList typeList;

	QMutexLocker dbLock (&databaseMutex);
	int result=db.list_types(typeList);
	dbLock.unlock ();

	// TODO generate from planes?
	if (db_ok==result)
	{
		QMutexLocker lock (&dataMutex);
		planeTypes=typeList;
		lock.unlock ();
	}

	return result;
}

bool retryOnResult (int result)
{
	if (result==db_err_timeout) return true;
	if (result==db_err_connection_failed) return true;
	return false;
}

bool DataStorage::refreshAll (OperationMonitor *monitor) throw ()
{
	// Refresh planes and people before refreshing flights!
	// TODO flightsOther
#define progress(p) do { if (monitor->isCanceled ()) return false; monitor->progress (p, 9); } while (0)
#define retrying(call) do { int result=call; if (!retryOnResult (result)) break; DefaultQThread::sleep (1); } while (1)
#define step(prog,text,call) do { progress (prog); monitor->status (text); retrying (call); } while (0)

	// TODO handle failing with !retryOnResult
	step (0, "Flugzeuge aktualisieren"          , refreshPlanes          ());
	step (1, "Personen aktualisieren"           , refreshPeople          ());
	step (2, "Startarten aktualisieren"         , refreshLaunchTypes     ());
	step (3, "Flüge aktualisieren"              , refreshFlightsToday    ());
	step (4, "Vorbereitete Flüge aktualisieren" , refreshPreparedFlights ());
	step (5, "Flugplätze aktualiseren"          , refreshAirfields       ());
	step (6, "Abrechnungshinweise aktualisieren", refreshAccountingNotes ());
	step (7, "Vereine aktualisieren"            , refreshClubs           ());
	step (8, "Flugzeugtypen aktualisieren"      , refreshPlaneTypes      ());
	progress (9);
#undef step
#undef progress

//	if (monitor->isCanceled ()) return false; monitor->progress (0, 9); monitor->status("Flugzeuge aktualisieren");           refreshPlanes ();
//	if (monitor->isCanceled ()) return false; monitor->progress (1, 9); monitor->status("Personen aktualisieren");            refreshPeople ();
//	if (monitor->isCanceled ()) return false; monitor->progress (2, 9); monitor->status("Startarten aktualisieren");          refreshLaunchTypes ();
//	if (monitor->isCanceled ()) return false; monitor->progress (3, 9); monitor->status("Flüge aktualisieren");               refreshFlightsToday ();
//	if (monitor->isCanceled ()) return false; monitor->progress (4, 9); monitor->status("Vorbereitete Flüge aktualisieren");  refreshPreparedFlights ();
//	if (monitor->isCanceled ()) return false; monitor->progress (5, 9); monitor->status("Flugplätze aktualiseren");           refreshAirfields ();
//	if (monitor->isCanceled ()) return false; monitor->progress (6, 9); monitor->status("Abrechnungshinweise aktualisieren"); refreshAccountingNotes ();
//	if (monitor->isCanceled ()) return false; monitor->progress (7, 9); monitor->status("Vereine aktualisieren");             refreshClubs ();
//	if (monitor->isCanceled ()) return false; monitor->progress (8, 9); monitor->status("Flugzeugtypen aktualisieren");       refreshPlaneTypes ();
//	if (monitor->isCanceled ()) return false; monitor->progress (9, 9);

	return true;
}


bool DataStorage::fetchFlightsDate (OperationMonitor *monitor, QDate date, bool *success, QString *message)
{
	monitor->status ("Flüge abrufen");

	if (monitor->isCanceled ()) return false;

	int result=refreshFlights (date, flightsOther, &otherDate);
	if (success) *success=(result==db_ok);

	std::cout << "result from refrechFlights(date) is " << result << std::endl;

	return true;
}


// *****************
// ** Data access **
// *****************

QList<Plane> DataStorage::getPlanes ()
{
	QMutexLocker lock (&dataMutex);
	return planes;
}

QStringList DataStorage::getPlaneRegistrations ()
{
	// TODO cache it, and generate only when needed
	QStringList result;

	QMutexLocker lock (&dataMutex);
	foreach (const Plane &plane, planes)
		result.append (plane.registration);
	lock.unlock ();

	result.sort ();

	return result;
}

QList<Person> DataStorage::getPeople ()
{
	QMutexLocker lock (&dataMutex);
	return people;
}

QList<LaunchType> DataStorage::getLaunchTypes ()
{
	QMutexLocker lock (&dataMutex);
	return launchTypes;
}

QStringList DataStorage::getAirfields ()
{
	QMutexLocker lock (&dataMutex);
	return airfields;
}

QStringList DataStorage::getAccountingNotes ()
{
	QMutexLocker lock (&dataMutex);
	return accountingNotes;
}

QList<Flight> DataStorage::getFlightsToday ()
{
	// TODO if today is not the real today, refresh today
	QMutexLocker lock (&dataMutex);
	return flightsToday.getList ();
}

QList<Flight> DataStorage::getFlightsOther ()
{
	QMutexLocker lock (&dataMutex);
	return flightsOther.getList ();
}

QList<Flight> DataStorage::getPreparedFlights ()
{
	QMutexLocker lock (&dataMutex);
	return preparedFlights.getList ();
}

QStringList DataStorage::getPersonFirstNames ()
{
	// TODO cache
	// TODO case insensitive
	QSet<QString> firstNames;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		firstNames.insert (person.vorname);
	lock.unlock ();

	// TODO sort case insensitively
	QStringList result=QStringList::fromSet (firstNames);
	result.sort ();
	return result;
}

QStringList DataStorage::getPersonFirstNames (const QString &lastName)
{
	// TODO cache
	QSet<QString> firstNames;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.nachname.toLower ()==lastName.toLower ())
			firstNames.insert (person.vorname);
	lock.unlock ();

	// TODO sort case insensitively
	QStringList result=QStringList::fromSet (firstNames);
	result.sort ();
	return result;
}

QStringList DataStorage::getPersonLastNames ()
{
	// TODO cache
	QSet<QString> lastNames;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		lastNames.insert (person.nachname);
	lock.unlock ();

	// TODO sort case insensitively
	QStringList result=QStringList::fromSet (lastNames);
	result.sort ();
	return result;
}

QStringList DataStorage::getPersonLastNames (const QString &firstName)
{
	// TODO cache
	// TODO case insensitive
	QSet<QString> lastNames;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.vorname.toLower ()==firstName.toLower ())
			lastNames.insert (person.nachname);
	lock.unlock ();

	// TODO sort case insensitively
	QStringList result=QStringList::fromSet (lastNames);
	result.sort ();
	return result;
}

QStringList DataStorage::getClubs ()
{
	QMutexLocker lock (&dataMutex);
	return clubs;
}

QStringList DataStorage::getPlaneTypes ()
{
	QMutexLocker lock (&dataMutex);
	return planeTypes;
}


// *****************
// ** Hash access **
// *****************

db_id DataStorage::getPlaneIdByRegistration (const QString &registration)
{
	// TODO implement with hash
	QMutexLocker lock (&dataMutex);
	foreach (const Plane &plane, planes)
		if (plane.registration.toLower ()==registration.toLower ())
			return plane.id;

	return invalid_id;
}

QList<db_id> DataStorage::getPersonIdsByName (const QString &firstName, const QString &lastName)
{
	// TODO implement with hash?
	QList<db_id> result;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.vorname.toLower ()==firstName.toLower () && person.nachname.toLower ()==lastName.toLower ())
			result.append (person.id);

	return result;
}

db_id DataStorage::getUniquePersonIdByName (const QString &firstName, const QString &lastName)
{
	// TODO implement with hash?
	// TODO: instead of getting all matching people, we could stop on the first
	// duplicate.
	const QList<db_id> personIds=getPersonIdsByName (firstName, lastName);

	if (personIds.size ()==1)
		return personIds.at (0);
	else
		return invalid_id;
}


QList<db_id> DataStorage::getPersonIdsByFirstName (const QString &firstName)
{
	// TODO implement with hash?
	QList<db_id> result;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.vorname.toLower ()==firstName.toLower ())
			result.append (person.id);
	lock.unlock ();

	return result;
}

QList<db_id> DataStorage::getPersonIdsByLastName (const QString &lastName)
{
	// TODO implement with hash?
	QList<db_id> result;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.nachname.toLower ()==lastName.toLower ())
			result.append (person.id);
	lock.unlock ();

	return result;
}



/**
 * Gets an object
 *
 * @param T the class of object
 * @param id the ID of the object
 * @return the object
 * @throw NotFoundException if the object is not found
 */
template<class T> T DataStorage::getObject (db_id id)
{
	if (id_invalid (id))
		throw NotFoundException (id);

	QMutexLocker lock (&dataMutex);
	foreach (const T &object, *objectList<T> ())
		if (object.get_id ()==id)
			return T (object);
	lock.unlock ();

	throw NotFoundException (id);
}

template<class T> bool DataStorage::objectExists (db_id id)
{
	QMutexLocker lock (&dataMutex);
	foreach (const T &object, *objectList<T> ())
		if (object.get_id ()==id)
			return true;
	lock.unlock ();

	return false;
}


// Different specialization for Flight
template<> Flight DataStorage::getObject (db_id id)
{
	// TODO search all flights if not found today/display date/prepared?

	QMutexLocker lock (&dataMutex);

	// TODO use EntityList methods

	if (todayDate.isValid ())
		foreach (const Flight &flight, flightsToday.getList ())
			if (flight.id==id)
				return Flight (flight);

	if (otherDate.isValid ())
		foreach (const Flight &flight, flightsOther.getList ())
		if (flight.id==id)
				return Flight (flight);

	foreach (const Flight &flight, preparedFlights.getList ())
		if (flight.id==id)
			return Flight (flight);
std::cout << "not found" << std::endl;
	throw NotFoundException (id);
}


QList<Person> DataStorage::getPeople (const QList<db_id> &ids)
{
	QList<Person> result;

	foreach (db_id id, ids)
		result.append (getObject<Person> (id));

	return result;
}

db_id DataStorage::getLaunchTypeByType (startart_type type)
{
	QMutexLocker lock (&dataMutex);
	foreach (const LaunchType &launchType, launchTypes)
		if (launchType.get_type ()==type)
			return launchType.get_id ();
	lock.unlock ();

	return invalid_id;
}

// Instantiate the get method templates
template Flight     DataStorage::getObject (db_id id);
template Plane      DataStorage::getObject (db_id id);
template Person     DataStorage::getObject (db_id id);
template LaunchType DataStorage::getObject (db_id id);

template Flight     *DataStorage::getNewObject (db_id id);
template Plane      *DataStorage::getNewObject (db_id id);
template Person     *DataStorage::getNewObject (db_id id);
template LaunchType *DataStorage::getNewObject (db_id id);



// **********************
// ** Helper templates **
// **********************

// Specialize the helper templates
// For Flight, there is no single list
template<> QList<Plane     > *DataStorage::objectList<Plane     > () { return &planes      ; }
template<> QList<Person    > *DataStorage::objectList<Person    > () { return &people      ; }
template<> QList<LaunchType> *DataStorage::objectList<LaunchType> () { return &launchTypes ; }



// **********************
// ** Database writing **
// **********************

// These methods are a bit ugly because of the interface to Database

// TODO:
//  - also addObject planeTypes, clubs, but unique => QSet?
//  - for flights: only if really today
//  - error check for getObject
//  - when a database operation fails (due to a lost connection) after the
//    query has been executed, the cache may be inconsistent.
//  - should success and message be a method of monitor?
template<class T> bool DataStorage::addObject (OperationMonitor *monitor, const T &object, db_id *id, bool *success, QString *message)
{
	(void)monitor;
	// TODO addObject duplicate check?

	// Make a copy of the object and set its ID to 0 (bad Database::write_x
	// interface)
	T copy (object);
	copy.id=0;

	// Write the object by using the database method
	QMutexLocker dbLock (&databaseMutex);
	// TODO error check
	db_id newId=db.writeObject (&copy);
	dbLock.unlock ();

	// TODO when can we cancel? When the object has been added, the cache
	// must be updated.
	//	if (monitor->isCanceled ()) return false;


	// If adding succeeded, add the object to the cache and emit a event
	if (id_valid (newId))
	{
		copy.id=newId;
		// TODO: we should read the object back - but what if adding succeeds
		// and reading back fails?
		// Read the object back
//		dbLock.relock ();
//		db.getObject (&copy, newId);
//		dbLock.unlock ();

		objectAdded (copy);

		// Emit a dbEvent
		emit dbEvent (DbEvent (det_add, DbEvent::getDbEventTable<T> (), newId));
	}

	// Task completed
	if (id) *id=newId;
	if (success) *success=(id_valid (newId));
	if (message) *message=db.get_last_error ();
	return true;
}

template<class T> bool DataStorage::deleteObject (OperationMonitor *monitor, db_id id, bool *success, QString *message)
{
	(void)monitor;

	// TODO error handling
	QMutexLocker dbLock (&databaseMutex);
	int result=db.deleteObject<T> (id);
	dbLock.unlock ();

	// TODO when can we cancel? When the object has been deleted, the cache
	// must be updated.
	//	if (monitor->isCanceled ()) return false;

	if (success) *success=(result==db_ok);
	if (message) *message=db.get_last_error ();

	if (result==db_ok)
	{
		objectDeleted<T> (id);

		// Emit a dbEvent
		emit dbEvent (DbEvent (det_delete, DbEvent::getDbEventTable<T> (), id));
	}

	// Task completed
	return true;
}

template<class T> bool DataStorage::updateObject (OperationMonitor *monitor, const T &object, bool *success, QString *message)
{
	(void)monitor;

	if (id_invalid (object.id)) return true; // TODO signal error
	T copy (object);

	QMutexLocker dbLock (&databaseMutex);
	db_id result=db.writeObject (&copy);
	dbLock.unlock ();


	// TODO when can we cancel? When the object has been updated, the cache
	// must be updated.
	//	if (monitor->isCanceled ()) return false;

	if (success) *success=id_valid (result);
	if (message) *message=db.get_last_error ();

	if (id_valid (result))
	{
		objectUpdated (copy);
		emit dbEvent (DbEvent (det_change, DbEvent::getDbEventTable<T> (), object.id));
	}


	return true;
}


// Instantiate the write method templates
template bool DataStorage::addObject (OperationMonitor *monitor, const Flight &, db_id *id, bool *success, QString *message);
template bool DataStorage::addObject (OperationMonitor *monitor, const Plane  &, db_id *id, bool *success, QString *message);
template bool DataStorage::addObject (OperationMonitor *monitor, const Person &, db_id *id, bool *success, QString *message);

template bool DataStorage::updateObject (OperationMonitor *monitor, const Flight &, bool *success, QString *message);
template bool DataStorage::updateObject (OperationMonitor *monitor, const Plane  &, bool *success, QString *message);
template bool DataStorage::updateObject (OperationMonitor *monitor, const Person &, bool *success, QString *message);

template bool DataStorage::deleteObject<Flight> (OperationMonitor *monitor, db_id id, bool *success, QString *message);
template bool DataStorage::deleteObject<Plane>  (OperationMonitor *monitor, db_id id, bool *success, QString *message);
template bool DataStorage::deleteObject<Person> (OperationMonitor *monitor, db_id id, bool *success, QString *message);


// **********************
// ** Database changes **
// **********************

// This template is specialized for T==Flight
template<class T> void DataStorage::objectAdded (const T &object)
{
	// Add the object to the cache
	QMutexLocker lock (&dataMutex);
	objectList<T> ()->append (object);
	lock.unlock ();
}

// This template is specialized for T==Flight
template<class T> void DataStorage::objectDeleted (db_id id)
{
	// Remove the object from the cache
	QList<T> *list=objectList<T> ();
	QMutableListIterator<T> it (*list);
	while (it.hasNext ())
		if (it.next ().get_id ()==id)
			it.remove ();
}

// This template is specialized for T==Flight
template<class T> void DataStorage::objectUpdated (const T &object)
{
	// TODO if the object is not in the cache, add it and log an error

	// Update the cache
	QList<T> *list=objectList<T> ();
	QMutableListIterator<T> it (*list);

	while (it.hasNext ())
		if (it.next ().get_id ()==object.get_id ())
			it.setValue (object);
}

// Specialize the change methods for Flight - we keep multiple lists here

template<> void DataStorage::objectAdded<Flight> (const Flight &flight)
{
	QMutexLocker lock (&dataMutex);

	if (flight.vorbereitet ())
		preparedFlights.append (flight);
	else if (flight.effdatum ()==todayDate)
		flightsToday.append (flight);
	else if (flight.effdatum ()==otherDate)
		// If otherDate is the same as today, this is not reached.
		flightsOther.append (flight);
	//else
	//	we're not interested in this flight
}

template<> void DataStorage::objectDeleted<Flight> (db_id id)
{
	// If any of the lists contain this flight, remove it
	preparedFlights.removeById (id);
	flightsToday.removeById (id);
	flightsOther.removeById (id);
}

template<> void DataStorage::objectUpdated<Flight> (const Flight &flight)
{
	// If the date or the prepared status of a flight changed, we may have to
	// relocate it to a different list. If the date is changed, it may not be
	// on any list at all any more; or it may not have been on any list before
	// (although the UI does not provide a way to modify a flight that is not
	// on one of these lists, but something like that may well be added, and
	// even if not, we'd still have to handle this case).

	// Determine which list the flight should be in (or none). Replace it if
	// it already exists, add it if not, and remove it from the other lists.
	if (flight.vorbereitet ())
	{
		preparedFlights.replaceOrAdd (flight.id, flight);
		flightsToday.removeById (flight.id);
		flightsOther.removeById (flight.id);
	}
	else if (flight.effdatum ()==todayDate)
	{
		preparedFlights.removeById (flight.id);
		flightsToday.replaceOrAdd (flight.id, flight);
		flightsOther.removeById (flight.id);
	}
	else if (flight.effdatum ()==otherDate)
	{
		// If otherDate is the same as today, this is not reached.
		preparedFlights.removeById (flight.id);
		flightsToday.removeById (flight.id);
		flightsOther.replaceOrAdd (flight.id, flight);
	}
	else
	{
		// The flight should not be on any list - remove it from all lists
		preparedFlights.removeById (flight.id);
		flightsToday.removeById (flight.id);
		flightsOther.removeById (flight.id);
	}
}


// Instantiate the change method templates (not for Flight - specialized)
template void DataStorage::objectAdded<Plane > (const Plane  &object);
template void DataStorage::objectAdded<Person> (const Person &object);

template void DataStorage::objectDeleted<Plane > (db_id id);
template void DataStorage::objectDeleted<Person> (db_id id);

template void DataStorage::objectUpdated<Plane > (const Plane  &plane );
template void DataStorage::objectUpdated<Person> (const Person &flight);


// ***********************
// ** Database querying **
// ***********************

/**
 *
 * @param monitor
 * @param id
 * @param result if not NULL, the result of the operation is written here
 * @return whether the operation completed, NOT the result of the operation!
 */
template<class T> bool DataStorage::objectUsed (OperationMonitor *monitor, db_id id, bool *result)
{
	if (id_invalid (id)) return false; // TODO signal error

	QMutexLocker dbLock (&databaseMutex);
	if (result) *result=db.objectUsed<T> (id);
	dbLock.unlock ();

	if (monitor->isCanceled ()) return false;

	return true;
}

// Instantiate the query methods
template bool DataStorage::objectUsed<Plane > (OperationMonitor *monitor, db_id id, bool *result);
template bool DataStorage::objectUsed<Person> (OperationMonitor *monitor, db_id id, bool *result);



/**
 *
 * @param id
 * @return a newly allocated copy of the object (the caller takes ownership),
 *         or NULL if id is invalid or not found
 */
template<class T> T* DataStorage::getNewObject (db_id id)
{
	try
	{
		return new T (getObject<T> (id));
	}
	catch (NotFoundException &ex)
	{
		return NULL;
	}
}


db_id DataStorage::planeFlying (db_id id)
{
	QMutexLocker lock (&dataMutex);
	// Only use the flights of today
	foreach (const Flight &flight, flightsToday.getList ())
		if (
			(flight.isFlying         () && flight.plane==id) ||
			(flight.isTowplaneFlying () && flight.towplane==id))
			return flight.id;

	return invalid_id;
}

db_id DataStorage::personFlying (db_id id)
{
	QMutexLocker lock (&dataMutex);
	// Only use the flights of today
	foreach (const Flight &flight, flightsToday.getList ())
		if (
			(flight.isFlying         () && flight.pilot    ==id) ||
			(flight.isFlying         () && flight.copilot==id) ||
			(flight.isTowplaneFlying () && flight.towpilot ==id))
			return flight.id;

	return invalid_id;
}

// ***************************
// ** Connection management **
// ***************************

void DataStorage::setState (DataStorage::State state)
{
	QMutexLocker lock (&dataMutex);
	bool changed=(state!=currentState);
	currentState=state;
	lock.unlock ();

	if (changed)
		emit stateChanged (state);
}

//void DataStorage::setAlive (bool alive)
//{
//	QMutexLocker lock (&dataMutex);
//	bool changed=(alive!=isAlive);
//	isAlive=alive;
//	lock.unlock ();
//
//	if (changed)
//	{
//		if (alive)
//			emit status ("OK", false);
//		else
//			emit status ("Verbindung unterbrochen", true);
//	}
//}

void DataStorage::connect ()
{
	worker.scheduleConnect ();
}

void DataStorage::disconnect ()
{
	worker.scheduleDisconnect ();
}


void DataStorage::doConnect ()
{
	setState (stateConnecting);

	bool connected=false; // Whether a connection is established at the moment
	bool unusable=false; // Whether the database has been found to be unusable

	// TODO allow cancelling
	while (!connected && !unusable)
	{
		try
		{
			QMutexLocker dbLock (&databaseMutex);
			db.connect (opts.server, opts.port, opts.username, opts.password);
			connected=true;
		}
		catch (OldDatabase::ex_access_denied      ) { unusable=true; }
		catch (OldDatabase::ex_connection_failed  ) { /* Keep trying */ }
		// TODO other exceptions

		// If we are not connected, wait one second before retrying
		DefaultQThread::sleep (1);
	}

	// If the connection succeeded (i. e. we are connected now), try to use the
	// database
	bool checked=false;
	while (!checked && !unusable)
	{
		if (connected)
		{
			try
			{
				QMutexLocker dbLock (&databaseMutex);
				db.use_db (opts.database);
				db.check_usability ();
				checked=true;
			}
			catch (OldDatabase::ex_access_denied          ) { unusable=true; }
			catch (OldDatabase::ex_database_not_found     ) { unusable=true; }
			catch (OldDatabase::ex_database_not_accessible) { unusable=true; }
			catch (OldDatabase::ex_insufficient_access    ) { unusable=true; }
			catch (OldDatabase::ex_unusable               ) { unusable=true; }
			catch (OldDatabase::ex_query_failed           ) { /* Keep trying */ }
			catch (OldDatabase::ex_timeout                ) { /* Keep trying */ }
			catch (OldDatabase::ex_connection_failed      ) { /* Keep trying */ }
			// TODO other exceptions
		}

		// If checking failed, wait one second before retrying
		DefaultQThread::sleep (1);
	}

	if (unusable)
	{
		if (connected)
			db.disconnect ();

		setState (stateUnusable);
	}
	else if (connected)
	{
		emit status ("Daten werden gelesen...", false);
		// TODO: pass an operation monitor to display the status in the main window
		SimpleOperationMonitor monitor;

		refreshAll (&monitor);

		setState (stateConnected);
	}
	else
	{
		// Canceled
	}
}


void DataStorage::doDisconnect ()
{
	emit status ("Verbindung zur Datenbank wird getrennt...", false);

	QMutexLocker dbLock (&databaseMutex);
	db.disconnect ();
	dbLock.unlock ();

	setState (stateOffline);
}

QString DataStorage::stateGetText (DataStorage::State state)
{
	switch (state)
	{
		case stateOffline: return "Nicht verbunden";
		case stateConnecting: return "Verbindungsaufbau...";
		case stateConnected: return "OK";
		case stateUnusable: return "Unbenutzbar";
		case stateLost: return "Verbindung unterbrochen";
	}

	assert (false);
	return "";
}

bool DataStorage::stateIsError (DataStorage::State state)
{
	switch (state)
	{
		case stateOffline: return false;
		case stateConnecting: return false;
		case stateConnected: return false;
		case stateUnusable: return true;
		case stateLost: return true;
	}

	assert (false);
	return false;
}

bool DataStorage::ping ()
{
	QMutexLocker dbLock (&databaseMutex);
	int result=db.ping ();
	dbLock.unlock ();

	return result==0;
}

bool DataStorage::isConnectionEstablished () const
{
	State state=getState ();

	return (state==stateConnected || state==stateLost);
}

bool DataStorage::isConnectionAlive () const
{
	return getState ()==stateConnected;
}

// **********
// ** Test **
// **********

bool DataStorage::sleep (OperationMonitor &monitor, int seconds)
{
	std::cout << "DataStorage waiting for " << seconds << " seconds on " << (isGuiThread ()?"the GUI thread":"a background thread") << std::endl;

	int i;
	for (i=0; i<seconds && !monitor.isCanceled (); ++i)
	{
		monitor.progress (i, seconds);
		DefaultQThread::msleep (1000);
	}

	return (i==seconds);
}

// ***************************
// ** Background processing **
// ***************************

void DataStorage::addTask (Task *task)
{
	worker.addTask (task);
}
