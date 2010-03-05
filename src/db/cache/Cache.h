/*
 * Cache.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <QObject>
#include <QDate>
#include <QList>
#include <QMap>
#include <QMutex>

#include "src/db/dbId.h"
#include "src/model/LaunchMethod.h" // Required for LaunchMethod::Type
#include "src/model/objectList/EntityList.h"
#include "src/db/event/DbEvent.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"

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
		 * The Cache tracks changes to the Database by the Event::Events emitted by
		 * the database. It also emits Event::Events after the cache contents
		 * change. Classes using the cache should listen for Event::Events from the
		 * cache rather than from the database.
		 *
		 * The QLists returned by the methods of this class are implicitly
		 * shared by Qt, so the data is not copied until the lists are modified
		 * or accessed by operator[] or a non-const iterator. If a list is not
		 * to be modified, it is recommended to declare it as const (e. g. const
		 * List<Plane>=dataStorage.getPlanes ()) to prevent accidental
		 * modifications which would cause the list data to be copied.
		 *
		 * This class is thread safe, provided that the database is thread safe
		 * (that is, accesses to the database are not synchronized).
		 */
		class Cache: public QObject
		{
			Q_OBJECT

			public:
				class NotFoundException: public std::exception
				{
					public:
						NotFoundException (dbId id): id (id) {}
						dbId id;
				};

				// *** Construction
				Cache (Database &db);
				virtual ~Cache ();
				void clear ();

				// *** Properties
				Database &getDatabase ();

				QDate getTodayDate ();
				QDate getOtherDate ();


				// *** Object lists
				EntityList<Plane> getPlanes ();
				EntityList<Person> getPeople ();
				EntityList<LaunchMethod> getLaunchMethods ();
				EntityList<Flight> getFlightsToday ();
				EntityList<Flight> getFlightsOther ();
				EntityList<Flight> getPreparedFlights ();
				template<class T> EntityList<T> getObjects () const;


				// *** Individual objects
				template<class T> T getObject (dbId id);
				template<class T> T* getNewObject (dbId id);
				template<class T> bool objectExists (dbId id);

				// TODO template
				QList<Person> getPeople (const QList<dbId> &ids);


				// *** Object lookup
				// TODO cache, sort, maps
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
				int refreshFlights ();
				int refreshFlightsToday ();
				int refreshFlightsOther ();
				int fetchFlightsOther (QDate date, OperationMonitorInterface monitor=OperationMonitorInterface::null);
				int refreshPreparedFlights ();

				int refreshLocations ();
				int refreshAccountingNotes ();

				bool refreshAll (OperationMonitorInterface monitor=OperationMonitorInterface::null);


				// *** Change handling
				template<class T> void objectAdded (const T &object);
				template<class T> void objectDeleted (dbId id);
				template<class T> void objectUpdated (const T &object);

				// TODO remove id methods after the object is passed in the
				// event
				template<class T> void objectAdded (dbId id);
				template<class T> void objectUpdated (dbId id);

			signals:
				void changed (Db::Event::DbEvent event);  // full type name


			protected:
				// *** Object lists
				// Helper templates, specialized in implementation
				template<class T> const EntityList<T> *objectList () const;
				template<class T> EntityList<T> *objectList ();

			protected slots:
				void dbChanged (Db::Event::DbEvent event); // full type name


			private:
				// *** Database
				Database &db;


				// *** Object lists

				// Note: when adding something here, also clear it in clear ().

				// Note that we cannot use an AutomaticEntityList here because that
				// accesses the database to retrieve the object identified by the ID
				// from the dbEvent, so the object must be in the cache before the
				// dbEvent is emitted.
				EntityList<Plane> planes;
				EntityList<Person> people;
				EntityList<LaunchMethod> launchMethods;

				EntityList<Flight> flightsToday; QDate todayDate;
				EntityList<Flight> flightsOther; QDate otherDate;
				EntityList<Flight> preparedFlights;

				// *** Object data
				QStringList locations;
				QStringList accountingNotes;
				QStringList clubs;
				QStringList planeTypes;


				// *** Concurrency
				// Improvement: use separate locks for flights, people...
				/** Locks accesses to data of this Cache */
				mutable QMutex dataMutex;


		};
	}
}

#endif
