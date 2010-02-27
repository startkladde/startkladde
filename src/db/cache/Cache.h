/*
 * Cache.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <QDate>
#include <QMutexLocker> // remove
#include <QList>

#include "src/db/dbId.h"
#include "src/model/LaunchMethod.h" // Required for LaunchMethod::Type

class Flight;
class Person;
class Plane;
template<class T> class EntityList;

namespace Db
{
	class Database;

	namespace Cache
	{
		/**
		 * A cache for a Database
		 *
		 * Caches the raw object as well as data generated from the object
		 * lists (e. g. the list of clubs) and other data explicitly read from
		 * the database (e. g. the list of locations).
		 *
		 * The Cache tracks changes to the Database by the DbEvents emitted by
		 * the database. It also emits DbEvents after the cache contents
		 * change. Classes using the cache should listen for DbEvents from the
		 * cache rather than from the database.
		 *
		 * The QLists returned by the methods of this class are implicitly
		 * shared by Qt, so the data is not copied until the lists are modified
		 * or accessed by operator[] or a non-const iterator. If a list is not
		 * to be modified, it is recommended to declare it as const (e. g. const
		 * List<Plane>=dataStorage.getPlanes ()) to prevent accidental
		 * modifications which would cause the list data to be copied.
		 *
		 * This class is thread safe.
		 */
		class Cache
		{
			class NotFoundException: public std::exception
			{
				public:
					NotFoundException (dbId id): id (id) {}
					dbId id;
			};

			public:
				// *** Construction
				Cache (Database &db);
				virtual ~Cache ();


				// *** Properties
				QDate getTodayDate ();
				QDate getOtherDate ();


				// *** Object lists
				// TODO these methods should return EntityList instead of QList
				QList<Plane> getPlanes ();
				QList<Person> getPeople ();
				QList<LaunchMethod> getLaunchMethods ();
				QList<Flight> getFlightsToday ();
				QList<Flight> getFlightsOther ();
				QList<Flight> getPreparedFlights ();
				template<class T> const QList<T> getObjects () const;


				// *** Individual objects
				template<class T> T getObject (dbId id);
				template<class T> T* getNewObject (dbId id);
				template<class T> bool objectExists (dbId id);

				// TODO template
				QList<Person> getPeople (const QList<dbId> &ids);


				// *** Object lookup
				// TODO cache, sort, hashes
				dbId getPlaneIdByRegistration (const QString &registration);
				QList<dbId> getPersonIdsByName (const QString &firstName, const QString &lastName);
				dbId getUniquePersonIdByName (const QString &firstName, const QString &lastName);
				QList<dbId> getPersonIdsByFirstName (const QString &firstName);
				QList<dbId> getPersonIdsByLastName (const QString &lastName);
				dbId getLaunchMethodByType (LaunchMethod::Type type);


				// *** Object data
				QStringList getPlaneRegistrations ();
				QStringList getPersonFirstNames ();
				QStringList getPersonFirstNames (const QString &lastName);
				QStringList getPersonLastNames ();
				QStringList getPersonLastNames (const QString &firstName);
				QStringList getLocations ();
				QStringList getAccountingNotes ();
				QStringList getPlaneTypes ();
				QStringList getClubs ();

				dbId planeFlying (dbId id);
				dbId personFlying (dbId id);


				// *** Reading
				// TODO allow canceling (old OperationMonitor)
				// TODO void
				int refreshPlanes ();
				int refreshPeople ();
				int refreshLaunchMethods ();
				int refreshFlightsToday ();
				int refreshFlightsOther ();
				int fetchFlightsOther (QDate date);
				int refreshPreparedFlights ();

				int refreshLocations ();
				int refreshAccountingNotes ();

				bool refreshAll ();


				// *** Change handling
				template<class T> void objectAdded (const T &object);
				template<class T> void objectDeleted (dbId id);
				template<class T> void objectUpdated (const T &object);


			protected:
				// *** Object lists
				// Helper templates, specialized in implementation
				template<class T> QList<T> *objectList ();

			private:
				// *** Database
				Database &db;


				// *** Object lists
				// Note that we cannot use an AutomaticEntityList here because that
				// accesses the database to retrieve the object identified by the ID
				// from the dbEvent - so the object must be in the cache before the
				// dbEvent is emitted!
				// TODO but we could use EntityList here; but note that it's not

				QList<Plane> planes;
				QList<Person> people;
				QList<LaunchMethod> launchMethods;

				// For flights, we keep multiple lists - one for the flights of today,
				// one for the flights of another date, and one for prepared flights.
				// The flights of today are required for planeFlying and personFlying.
				// In the future, we may add lists for an arbitrary number of dates.
				// TODO why pointers?
				EntityList<Flight> *flightsToday; QDate todayDate;
				EntityList<Flight> *flightsOther; QDate otherDate;
				EntityList<Flight> *preparedFlights;

				// *** Object data
				QStringList locations;
				QStringList accountingNotes;
				QStringList clubs;
				QStringList planeTypes;

				// *** Concurrency
				// TODO locks for different object classes?
				/** Locks accesses to data of this Cache */
				mutable QMutex dataMutex;


		};
	}
}

#endif
