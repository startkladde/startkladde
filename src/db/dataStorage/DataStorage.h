#ifndef DATASTORAGE_H_
#define DATASTORAGE_H_

#include <cassert>

#include <QObject>
#include <QList>
#include <QStringList>
#include <QString>
#include <QMap>
#include <QMutexLocker>
#include <QDate>

#include "src/db/dbId.h"
#include "src/db/DbEvent.h"
#include "src/model/LaunchMethod.h" // Required for LaunchMethod::Type

namespace Db
{
	class ThreadSafeDatabase;
}

class Task;
class Flight;
class Person;
class Plane;
class OperationMonitor;
class DataStorageWorker;
template<class T> class EntityList;

/**
 * A wrapper around Database
 *
 * This wrapper provides:
 *   - a better interfaces
 *   - caching
 *
 * This should be considered a temporary solution to facilitate cleanups of the
 * GUI classes, primarily the flight editor window. It will probably be merged
 * with the Database class some time in the future.
 *
 * When using this class, all write accesses to the database should be done
 * using this class. If not, the cache may be out of date. In that case,
 * the appropriate refresh methods must be called between write accesses to the
 * database not using this class and read accesses using this class.
 *
 * The QLists returned by the methods of this class are implicitly shared, so
 * the data is not copied until the lists are modified or accessed by
 * operator[] or a non-const iterator. If a list is not modified, it is
 * recommended to declare it as const (const List<T>=dataStorage.getTs ()) to
 * prevent accidental modifications which would cause the list (and its data)
 * to be copied.
 */
class DataStorage: public QObject
{
	Q_OBJECT

	public:
		class NotFoundException: public std::exception
		{
			public:
				NotFoundException (dbId id): id (id) {}
				dbId id;
		};

		friend class DataStorageMonitor;

		DataStorage (Db::ThreadSafeDatabase &db);
		virtual ~DataStorage ();

		int refreshPlanes ();
		int refreshPeople ();
		int refreshLaunchMethods ();
		int refreshFlights (QDate date, EntityList<Flight> &listTarget, QDate *dateTarget);
		int refreshFlightsToday ();
		int refreshFlightsOther ();
		int refreshPreparedFlights ();
		int refreshLocations ();
		int refreshAccountingNotes ();
		int refreshClubs ();
		int refreshPlaneTypes ();
		bool refreshAll (OperationMonitor *monitor) throw ();
		bool fetchFlightsDate (OperationMonitor *monitor, QDate date, bool *success, QString *message);

		QList<Plane> getPlanes ();
		QStringList getPlaneRegistrations ();
		QList<Person> getPeople ();
		QStringList getPersonFirstNames ();
		QStringList getPersonFirstNames (const QString &lastName);
		QStringList getPersonLastNames ();
		QStringList getPersonLastNames (const QString &firstName);
		QList<LaunchMethod> getLaunchMethods ();
		// TODO these methods should return EntityList instead of QList
		QList<Flight> getFlightsToday ();
		QList<Flight> getFlightsOther ();
		QList<Flight> getPreparedFlights ();
		QStringList getLocations ();
		QStringList getAccountingNotes ();
		QStringList getPlaneTypes ();
		QStringList getClubs ();
		QDate getDisplayDate ();
		QList<Flight> getFlightsDisplayDate ();

		QDate getTodayDate () { QMutexLocker lock (&dataMutex); return todayDate; }
		QDate getOtherDate () { QMutexLocker lock (&dataMutex); return otherDate; }

		// *** Hash Access
		// TODO cache, sort, hashes
		dbId getPlaneIdByRegistration (const QString &registration);
		QList<dbId> getPersonIdsByName (const QString &firstName, const QString &lastName);
		dbId getUniquePersonIdByName (const QString &firstName, const QString &lastName);
		QList<dbId> getPersonIdsByFirstName (const QString &firstName);
		QList<dbId> getPersonIdsByLastName (const QString &lastName);
		QList<Person> getPeople (const QList<dbId> &ids);
		dbId getLaunchMethodByType (LaunchMethod::Type type);

		dbId planeFlying (dbId id);
		dbId personFlying (dbId id);

		// *** Object getting
		template<class T> T getObject (dbId id);
		template<class T> T* getNewObject (dbId id);
		template<class T> bool objectExists (dbId id);
		// TODO the cast should be in the non-const method
		// TODO: make a copy of the list (synchronized) because the lists have to be locked for concurrent access
		template<class T> const QList<T> getObjects () const { return *(const_cast<DataStorage *> (this))->objectList<T> (); }

		// *** Database writing
		template<class T> bool addObject (OperationMonitor *monitor, const T &object, dbId *id, bool *success, QString *message);
		template<class T> bool updateObject (OperationMonitor *monitor, const T &object, bool *success, QString *message);
		template<class T> bool deleteObject (OperationMonitor *monitor, dbId id, bool *success, QString *message);

		// *** Database change handling
		template<class T> void objectAdded (const T &object);
		template<class T> void objectDeleted (dbId id);
		template<class T> void objectUpdated (const T &object);


		// *** Database querying
		template<class T> bool objectUsed (OperationMonitor *monitor, dbId id, bool *result);


		// *** Refreshing view
		template<class T> void refreshViews () { emit dbEvent (DbEvent (DbEvent::typeRefresh, DbEvent::getTable<T> (), invalidId)); }
		void refreshAllViews () { emit DbEvent (DbEvent::typeRefresh, DbEvent::tableAll, invalidId); }

		// *** Test methods
		bool sleep (OperationMonitor &monitor, int seconds);

		// *** Background processing
		void addTask (Task *task);


	protected:
		// Helper templates, specialized in implementation
		template<class T> QList<T> *objectList ();

	signals:
		void dbEvent (DbEvent event);
		void executingQuery (QString query);

	private:
		Db::ThreadSafeDatabase &db;

		// Note that we cannot use an AutomaticEntityList here because that
		// accesses the database to retrieve the object identified by the ID
		// from the dbEvent - so the object must be in the cache before the
		// dbEvent is emitted!
		// TODO but we could use EntityList here; but note that it's not

		QList<Plane> planes;
		QList<Person> people;
		QList<LaunchMethod> launchMethods;
		QStringList locations;
		QStringList accountingNotes;
		QStringList clubs; // TODO generate from planes+people
		QStringList planeTypes; // TODO generate from planes

		// For flights, we keep multiple lists - one for the flights of today,
		// one for the flights of another date, and one for prepared flights.
		// The flights of today are required for planeFlying and personFlying.
		// In the future, we may add lists for an arbitrary number of dates.
		EntityList<Flight> *flightsToday; QDate todayDate;
		EntityList<Flight> *flightsOther; QDate otherDate;
		EntityList<Flight> *preparedFlights;


		// ***************************
		// ** Connection management **
		// ***************************

		friend class DataStorageWorker;

	public:
		enum State
		{
			stateOffline,
			stateConnecting,
			stateConnected,
			stateUnusable,
			stateLost
		};

		bool isConnectionEstablished () const;
		bool isConnectionAlive () const;
		static QString stateGetText (DataStorage::State state);
		static bool stateIsError (DataStorage::State state);
		bool ping ();
		bool ping (int timeout);
		State getState () const { QMutexLocker lock (&dataMutex); return currentState; }
		void setState () {  }

	public slots:
		void connect ();
		void disconnect ();

	signals:
		void stateChanged (DataStorage::State state);
		void status (QString status, bool isError);

	protected:
		// *** Connection management
		void setState (State newState);
//		void setAlive (bool alive);

	private:
		State currentState;
		bool isAlive;
		DataStorageWorker *worker;

		mutable QMutex databaseMutex; // Locks accesses to db
		mutable QMutex dataMutex; // Locks accesses to data of this DataStorage

		void doConnect ();
		void doDisconnect ();

};

#endif
