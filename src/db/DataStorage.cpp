#include "DataStorage.h"

#include <iostream>

#include <QSet>
#include <QThread>
#include <QProgressDialog>
#include <QApplication>
#include <QMutableListIterator>

#include "src/concurrent/threadUtil.h"
#include "src/concurrent/DefaultQThread.h"
#include "src/concurrent/monitor/SimpleOperationMonitor.h"
#include "src/config/Options.h" // TODO remove dependency
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"
#include "src/db/Database.h"
#include "src/db/DbEvent.h"
#include "src/db/DataStorageWorker.h"
#include "src/model/objectList/EntityList.h"

using namespace std;

const int db_ok=0;

// FIXME all methods accessing the database:
//   - error handling
//   - proper locking (database, then data)
//   - return values


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

DataStorage::DataStorage (Database &db):
	db (db),
	flightsToday (new EntityList<Flight> ()),
	flightsOther (new EntityList<Flight> ()),
	preparedFlights (new EntityList<Flight> ()),
	currentState (stateOffline),
	worker (new DataStorageWorker (*this, 1000))
{
	// FIXME
//	QObject::connect (&db, SIGNAL (executing_query (QString)), this, SIGNAL (executingQuery (QString)));
	worker->start ();
}

DataStorage::~DataStorage ()
{
	// This causes the program to hang on quit when the server is unresponsive
//	worker->stop (true);

	// This is not nice, but it will have to do for now.
	// TODO this should not be in the destructor, a terminate method would be
	// better, so we can terminate all threads before waiting
	worker->setTerminationEnabled (true);
	worker->terminate ();
	std::cout << "Wating for data storage worker to terminate..."; std::cout.flush ();
	if (worker->wait (1000))
		std::cout << "done" << std::endl;
	else
		std::cout << "timeout" << std::endl;

	delete worker;
	delete flightsToday;
	delete flightsOther;
	delete preparedFlights;
}

// **********************
// ** Database reading **
// **********************

// FIXME can probably be removed
#define copyList(T, source, target) do \
{ \
	target.clear (); \
	foreach (T it, source) \
		target.append (it); \
} while (0)

// FIXME can probably be removed
#define copyListLocked(T, source, target) do \
{ \
	QMutexLocker lock (&dataMutex); \
	copyList (T, source, target); \
	lock.unlock (); \
} while (0)


int DataStorage::refreshPlanes ()
{

	QMutexLocker dbLock (&databaseMutex);
	QList<Plane> planeList=db.getObjects<Plane> ();
	dbLock.unlock ();

	copyListLocked (Plane, planeList, planes);

	return 0;
}

int DataStorage::refreshPeople ()
{
	QMutexLocker dbLock (&databaseMutex);
	QList<Person> personList=db.getObjects<Person> ();
	dbLock.unlock ();

	copyListLocked (Person, personList, people);

	return 0;
}

int DataStorage::refreshLaunchMethods ()
{
	QMutexLocker dbLock (&databaseMutex);
	QList<LaunchMethod> launchMethodList=db.getObjects<LaunchMethod> ();
	dbLock.unlock ();

	copyListLocked (LaunchMethod, launchMethodList, launchMethods);

	return 0;
}

int DataStorage::refreshFlights (QDate date, EntityList<Flight> &listTarget, QDate *dateTarget)
{
	QList<Flight> flightList;

	QMutexLocker dbLock (&databaseMutex);
	if (date.isNull ())
		flightList=db.getPreparedFlights ();
	else
		flightList=db.getFlightsDate (date);
	dbLock.unlock ();

	if (dateTarget) *dateTarget=QDate ();
	QMutexLocker lock (&dataMutex);
	copyList (Flight, flightList, listTarget);
	if (dateTarget) *dateTarget=date;
	lock.unlock ();

	return 0;
}

/**
 * Frontend to refreshFlights
 */
int DataStorage::refreshFlightsToday ()
{
	return refreshFlights (QDate::currentDate (), *flightsToday, &todayDate);
}

/**
 * Frontend to refreshFlights
 */
int DataStorage::refreshPreparedFlights ()
{
	return refreshFlights (QDate (), *preparedFlights, NULL);
}

int DataStorage::refreshAirfields ()
{
	QMutexLocker dbLock (&databaseMutex);
	QStringList airfieldList=db.listAirfields ();
	dbLock.unlock ();

	QMutexLocker lock (&dataMutex);
	airfields=airfieldList;
	lock.unlock ();

	return 0;
}

int DataStorage::refreshAccountingNotes ()
{
	QMutexLocker dbLock (&databaseMutex);
	QStringList accountingNoteList=db.listAccountingNotes ();
	dbLock.unlock ();

	QMutexLocker lock (&dataMutex);
	accountingNotes=accountingNoteList;
	lock.unlock ();

	return 0;
}

int DataStorage::refreshClubs ()
{
	QMutexLocker dbLock (&databaseMutex);
	QStringList clubList=db.listClubs ();
	dbLock.unlock ();

	QMutexLocker lock (&dataMutex);
	clubs=clubList;
	lock.unlock ();

	return 0;
}

int DataStorage::refreshPlaneTypes ()
{
	QMutexLocker dbLock (&databaseMutex);
	QStringList typeList=db.listPlaneTypes ();
	dbLock.unlock ();

	QMutexLocker lock (&dataMutex);
	planeTypes=typeList;
	lock.unlock ();

	return 0;
}

// FIXME
bool retryOnResult (int result)
{
//	if (result==db_err_timeout) return true;
//	if (result==db_err_connection_failed) return true;
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
	step (2, "Startarten aktualisieren"         , refreshLaunchMethods   ());
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
//	if (monitor->isCanceled ()) return false; monitor->progress (2, 9); monitor->status("Startarten aktualisieren");          refreshLaunchMethods ();
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

	int result=refreshFlights (date, *flightsOther, &otherDate);
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

QList<LaunchMethod> DataStorage::getLaunchMethods ()
{
	QMutexLocker lock (&dataMutex);
	return launchMethods;
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
	return flightsToday->getList ();
}

QList<Flight> DataStorage::getFlightsOther ()
{
	QMutexLocker lock (&dataMutex);
	return flightsOther->getList ();
}

QList<Flight> DataStorage::getPreparedFlights ()
{
	QMutexLocker lock (&dataMutex);
	return preparedFlights->getList ();
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

dbId DataStorage::getPlaneIdByRegistration (const QString &registration)
{
	// TODO implement with hash
	QMutexLocker lock (&dataMutex);
	foreach (const Plane &plane, planes)
		if (plane.registration.toLower ()==registration.toLower ())
			return plane.getId ();

	return invalidId;
}

QList<dbId> DataStorage::getPersonIdsByName (const QString &firstName, const QString &lastName)
{
	// TODO implement with hash?
	QList<dbId> result;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.vorname.toLower ()==firstName.toLower () && person.nachname.toLower ()==lastName.toLower ())
			result.append (person.getId ());

	return result;
}

dbId DataStorage::getUniquePersonIdByName (const QString &firstName, const QString &lastName)
{
	// TODO implement with hash?
	// TODO: instead of getting all matching people, we could stop on the first
	// duplicate.
	const QList<dbId> personIds=getPersonIdsByName (firstName, lastName);

	if (personIds.size ()==1)
		return personIds.at (0);
	else
		return invalidId;
}


QList<dbId> DataStorage::getPersonIdsByFirstName (const QString &firstName)
{
	// TODO implement with hash?
	QList<dbId> result;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.vorname.toLower ()==firstName.toLower ())
			result.append (person.getId ());
	lock.unlock ();

	return result;
}

QList<dbId> DataStorage::getPersonIdsByLastName (const QString &lastName)
{
	// TODO implement with hash?
	QList<dbId> result;

	QMutexLocker lock (&dataMutex);
	foreach (const Person &person, people)
		if (person.nachname.toLower ()==lastName.toLower ())
			result.append (person.getId ());
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
template<class T> T DataStorage::getObject (dbId id)
{
	if (idInvalid (id))
		throw NotFoundException (id);

	QMutexLocker lock (&dataMutex);
	foreach (const T &object, *objectList<T> ())
		if (object.getId ()==id)
			return T (object);
	lock.unlock ();

	throw NotFoundException (id);
}

template<class T> bool DataStorage::objectExists (dbId id)
{
	QMutexLocker lock (&dataMutex);
	foreach (const T &object, *objectList<T> ())
		if (object.getId ()==id)
			return true;
	lock.unlock ();

	return false;
}


// Different specialization for Flight
template<> Flight DataStorage::getObject (dbId id)
{
	// TODO search all flights if not found today/display date/prepared?

	QMutexLocker lock (&dataMutex);

	// TODO use EntityList methods

	if (todayDate.isValid ())
		foreach (const Flight &flight, flightsToday->getList ())
			if (flight.getId ()==id)
				return Flight (flight);

	if (otherDate.isValid ())
		foreach (const Flight &flight, flightsOther->getList ())
		if (flight.getId ()==id)
				return Flight (flight);

	foreach (const Flight &flight, preparedFlights->getList ())
		if (flight.getId ()==id)
			return Flight (flight);
std::cout << "not found" << std::endl;
	throw NotFoundException (id);
}


QList<Person> DataStorage::getPeople (const QList<dbId> &ids)
{
	QList<Person> result;

	foreach (dbId id, ids)
		result.append (getObject<Person> (id));

	return result;
}

dbId DataStorage::getLaunchMethodByType (LaunchMethod::Type type)
{
	QMutexLocker lock (&dataMutex);
	foreach (const LaunchMethod &launchMethod, launchMethods)
		if (launchMethod.type==type)
			return launchMethod.getId ();
	lock.unlock ();

	return invalidId;
}

// Instantiate the get method templates
template Flight     DataStorage::getObject (dbId id);
template Plane      DataStorage::getObject (dbId id);
template Person     DataStorage::getObject (dbId id);
template LaunchMethod DataStorage::getObject (dbId id);

template Flight     *DataStorage::getNewObject (dbId id);
template Plane      *DataStorage::getNewObject (dbId id);
template Person     *DataStorage::getNewObject (dbId id);
template LaunchMethod *DataStorage::getNewObject (dbId id);



// **********************
// ** Helper templates **
// **********************

// Specialize the helper templates
// For Flight, there is no single list
template<> QList<Plane     > *DataStorage::objectList<Plane     > () { return &planes      ; }
template<> QList<Person    > *DataStorage::objectList<Person    > () { return &people      ; }
template<> QList<LaunchMethod> *DataStorage::objectList<LaunchMethod> () { return &launchMethods ; }



// **********************
// ** Database writing **
// **********************

// These methods are a bit ugly because of the interface to Database
// TODO Database is better now

// TODO:
//  - also addObject planeTypes, clubs, but unique => QSet?
//  - for flights: only if really today
//  - error check for getObject
//  - when a database operation fails (due to a lost connection) after the
//    query has been executed, the cache may be inconsistent.
//  - should success and message be a method of monitor?
template<class T> bool DataStorage::addObject (OperationMonitor *monitor, const T &object, dbId *id, bool *success, QString *message)
{
	(void)monitor;
	// TODO addObject duplicate check?

	// The copy will have the same id as the original, but the id will be
	// overwritten.
	T copy (object);

	// Write the object by using the database method
	QMutexLocker dbLock (&databaseMutex);
	// TODO error check
	dbId newId=db.createObject (copy);
	dbLock.unlock ();

	// TODO when can we cancel? When the object has been added, the cache
	// must be updated.
	//	if (monitor->isCanceled ()) return false;


	// If adding succeeded, add the object to the cache and emit a event
	if (idValid (newId))
	{
		objectAdded (copy);
		emit dbEvent (DbEvent (DbEvent::typeAdd, DbEvent::getTable<T> (), newId));
	}

	// Task completed
	if (id) *id=newId;
	if (success) *success=(idValid (newId));
	if (message) *message=db.lastError ().text ();
	return true;
}

template<class T> bool DataStorage::deleteObject (OperationMonitor *monitor, dbId id, bool *success, QString *message)
{
	(void)monitor;

	// TODO error handling
	QMutexLocker dbLock (&databaseMutex);
	int affectedRows=db.deleteObject<T> (id);
	dbLock.unlock ();

	// TODO when can we cancel? When the object has been deleted, the cache
	// must be updated.
	//	if (monitor->isCanceled ()) return false;

	bool ok=(affectedRows>0);

	if (success) *success=ok;
	if (message) *message=db.lastError ().text ();

	if (ok)
	{
		objectDeleted<T> (id);

		// Emit a dbEvent
		emit dbEvent (DbEvent (DbEvent::typeDelete, DbEvent::getTable<T> (), id));
	}

	// Task completed
	return true;
}

template<class T> bool DataStorage::updateObject (OperationMonitor *monitor, const T &object, bool *success, QString *message)
{
	(void)monitor;

	if (idInvalid (object.getId ())) return true; // TODO signal error
	T copy (object);

	QMutexLocker dbLock (&databaseMutex);
	dbId result=db.updateObject (copy);
	dbLock.unlock ();


	// TODO when can we cancel? When the object has been updated, the cache
	// must be updated.
	//	if (monitor->isCanceled ()) return false;

	if (success) *success=idValid (result);
	if (message) *message=db.lastError ().text ();

	if (idValid (result))
	{
		objectUpdated (copy);
		emit dbEvent (DbEvent (DbEvent::typeChange, DbEvent::getTable<T> (), object.getId ()));
	}


	return true;
}


// Instantiate the write method templates
template bool DataStorage::addObject (OperationMonitor *monitor, const Flight       &, dbId *id, bool *success, QString *message);
template bool DataStorage::addObject (OperationMonitor *monitor, const Plane        &, dbId *id, bool *success, QString *message);
template bool DataStorage::addObject (OperationMonitor *monitor, const Person       &, dbId *id, bool *success, QString *message);
template bool DataStorage::addObject (OperationMonitor *monitor, const LaunchMethod &, dbId *id, bool *success, QString *message);

template bool DataStorage::updateObject (OperationMonitor *monitor, const Flight       &, bool *success, QString *message);
template bool DataStorage::updateObject (OperationMonitor *monitor, const Plane        &, bool *success, QString *message);
template bool DataStorage::updateObject (OperationMonitor *monitor, const Person       &, bool *success, QString *message);
template bool DataStorage::updateObject (OperationMonitor *monitor, const LaunchMethod &, bool *success, QString *message);

template bool DataStorage::deleteObject<Flight      > (OperationMonitor *monitor, dbId id, bool *success, QString *message);
template bool DataStorage::deleteObject<Plane       > (OperationMonitor *monitor, dbId id, bool *success, QString *message);
template bool DataStorage::deleteObject<Person      > (OperationMonitor *monitor, dbId id, bool *success, QString *message);
template bool DataStorage::deleteObject<LaunchMethod> (OperationMonitor *monitor, dbId id, bool *success, QString *message);


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
template<class T> void DataStorage::objectDeleted (dbId id)
{
	// Remove the object from the cache
	QList<T> *list=objectList<T> ();
	QMutableListIterator<T> it (*list);
	while (it.hasNext ())
		if (it.next ().getId ()==id)
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
		if (it.next ().getId ()==object.getId ())
			it.setValue (object);
}

// Specialize the change methods for Flight - we keep multiple lists here

template<> void DataStorage::objectAdded<Flight> (const Flight &flight)
{
	QMutexLocker lock (&dataMutex);

	if (flight.isPrepared ())
		preparedFlights->append (flight);
	else if (flight.effdatum ()==todayDate)
		flightsToday->append (flight);
	else if (flight.effdatum ()==otherDate)
		// If otherDate is the same as today, this is not reached.
		flightsOther->append (flight);
	//else
	//	we're not interested in this flight
}

template<> void DataStorage::objectDeleted<Flight> (dbId id)
{
	// If any of the lists contain this flight, remove it
	preparedFlights->removeById (id);
	flightsToday->removeById (id);
	flightsOther->removeById (id);
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
	if (flight.isPrepared ())
	{
		preparedFlights->replaceOrAdd (flight.getId (), flight);
		flightsToday->removeById (flight.getId ());
		flightsOther->removeById (flight.getId ());
	}
	else if (flight.effdatum ()==todayDate)
	{
		preparedFlights->removeById (flight.getId ());
		flightsToday->replaceOrAdd (flight.getId (), flight);
		flightsOther->removeById (flight.getId ());
	}
	else if (flight.effdatum ()==otherDate)
	{
		// If otherDate is the same as today, this is not reached.
		preparedFlights->removeById (flight.getId ());
		flightsToday->removeById (flight.getId ());
		flightsOther->replaceOrAdd (flight.getId (), flight);
	}
	else
	{
		// The flight should not be on any list - remove it from all lists
		preparedFlights->removeById (flight.getId ());
		flightsToday->removeById (flight.getId ());
		flightsOther->removeById (flight.getId ());
	}
}


// Instantiate the change method templates (not for Flight - specialized)
template void DataStorage::objectAdded<Plane       > (const Plane        &object);
template void DataStorage::objectAdded<Person      > (const Person       &object);
template void DataStorage::objectAdded<LaunchMethod> (const LaunchMethod &object);

template void DataStorage::objectDeleted<Plane       > (dbId id);
template void DataStorage::objectDeleted<Person      > (dbId id);
template void DataStorage::objectDeleted<LaunchMethod> (dbId id);

template void DataStorage::objectUpdated<Plane       > (const Plane        &plane );
template void DataStorage::objectUpdated<Person      > (const Person       &flight);
template void DataStorage::objectUpdated<LaunchMethod> (const LaunchMethod &flight);


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
template<class T> bool DataStorage::objectUsed (OperationMonitor *monitor, dbId id, bool *result)
{
	if (idInvalid (id)) return false; // TODO signal error

	QMutexLocker dbLock (&databaseMutex);
	// FIXME
//	if (result) *result=db.objectUsed<T> (id);
	if (result) *result=false;
	dbLock.unlock ();

	if (monitor->isCanceled ()) return false;

	return true;
}

// Instantiate the query methods
template bool DataStorage::objectUsed<Plane       > (OperationMonitor *monitor, dbId id, bool *result);
template bool DataStorage::objectUsed<Person      > (OperationMonitor *monitor, dbId id, bool *result);
template bool DataStorage::objectUsed<LaunchMethod> (OperationMonitor *monitor, dbId id, bool *result);



/**
 *
 * @param id
 * @return a newly allocated copy of the object (the caller takes ownership),
 *         or NULL if id is invalid or not found
 */
template<class T> T* DataStorage::getNewObject (dbId id)
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


dbId DataStorage::planeFlying (dbId id)
{
	QMutexLocker lock (&dataMutex);
	// Only use the flights of today
	foreach (const Flight &flight, flightsToday->getList ())
		if (
			(flight.isFlying         () && flight.planeId==id) ||
			(flight.isTowplaneFlying () && flight.towplaneId==id))
			return flight.getId ();

	return invalidId;
}

dbId DataStorage::personFlying (dbId id)
{
	QMutexLocker lock (&dataMutex);
	// Only use the flights of today
	foreach (const Flight &flight, flightsToday->getList ())
		if (
			(flight.isFlying         () && flight.pilotId    ==id) ||
			(flight.isFlying         () && flight.copilotId==id) ||
			(flight.isTowplaneFlying () && flight.towpilotId ==id))
			return flight.getId ();

	return invalidId;
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
	worker->scheduleConnect ();
}

void DataStorage::disconnect ()
{
	worker->scheduleDisconnect ();
}


void DataStorage::doConnect ()
{
	setState (stateConnecting);

	bool connected=false; // Whether a connection is established at the moment
	bool unusable=false; // Whether the database has been found to be unusable

	// TODO allow cancelling
	while (!connected && !unusable)
	{
//		try
//		{
			QMutexLocker dbLock (&databaseMutex);
			db.open (opts.databaseInfo);
			connected=true;
			dbLock.unlock ();
//		}
		// FIXME
//		catch (OldDatabase::ex_access_denied      ) { unusable=true; }
//		catch (OldDatabase::ex_connection_failed  ) { /* Keep trying */ }
		// TODO other exceptions

		// If we are not connected, wait one second before retrying
		DefaultQThread::sleep (1);
	}

	// FIXME
//	// If the connection succeeded (i. e. we are connected now), try to use the
//	// database
//	bool checked=false;
//	while (!checked && !unusable)
//	{
//		if (connected)
//		{
//			try
//			{
//				QMutexLocker dbLock (&databaseMutex);
//				db.use_db (opts.database);
//				db.check_usability ();
//				checked=true;
//			}
//			catch (OldDatabase::ex_access_denied          ) { unusable=true; }
//			catch (OldDatabase::ex_database_not_found     ) { unusable=true; }
//			catch (OldDatabase::ex_database_not_accessible) { unusable=true; }
//			catch (OldDatabase::ex_insufficient_access    ) { unusable=true; }
//			catch (OldDatabase::ex_unusable               ) { unusable=true; }
//			catch (OldDatabase::ex_query_failed           ) { /* Keep trying */ }
//			catch (OldDatabase::ex_timeout                ) { /* Keep trying */ }
//			catch (OldDatabase::ex_connection_failed      ) { /* Keep trying */ }
//			// TODO other exceptions
//		}
//
//		// If checking failed, wait one second before retrying
//		DefaultQThread::sleep (1);
//	}

	if (unusable)
	{
		if (connected)
			db.close ();

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
	db.close ();
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
	// FIXME
//	int result=db.ping ();
	int result=0;
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
	worker->addTask (task);
}
