/*
 * Implementation notes:
 *   - All accesses to internal data, even single values, must be protected by
 *     dataMutex. Use:
 *     - synchronizedReturn (dataMutex, value) if just a value is returned
 *     - synchronized (dataMutex) { ... } if this doesn't cause warnings about
 *       control reaching the end of the function (in methods which return a
 *       value)
 *     - QMutexLocker (&dataMutex) otherwise
 *     - dataMutex.lock () only when there's good reason (and document the
 *       reason)
 */

/*
 * TODO:
 *   - don't delete an entity that is still in use
 *   - club list should not include ""/whitespace only; Locations dito
 *   - Proper caching
 *     - ID->Object, Registration->Plane, FirstName->People (?)
 *     - Use maps: better insert; sorted
 *
 */

/*
 * Improvements:
 *   - log an error if an invalid ID is passed to the get by ID functions
 */

// TODO all methods accessing the database:
//   - error handling


/*
 * Cache.cpp
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#include "Cache.h"

#include <iostream>

#include <QSet>

#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/db/Database.h"
#include "src/concurrent/synchronized.h"
#include "src/util/qString.h"


namespace Db
{
	namespace Cache
	{
		Cache::Cache (Database &db):
			db (db)
		{
			connect (&db, SIGNAL (dbEvent (Db::Event::DbEvent)), this, SLOT (dbChanged (Db::Event::DbEvent)));
		}

		Cache::~Cache ()
		{
		}

		void Cache::clear ()
		{
			synchronized (dataMutex)
			{
				planes       .clear ();
				people       .clear ();
				launchMethods.clear ();

				flightsToday.clear (); todayDate=QDate ();
				flightsOther.clear (); otherDate=QDate ();
				preparedFlights.clear ();

				locations.clear ();
				accountingNotes.clear ();
				clubs.clear ();
				planeTypes.clear ();
			}
		}

		// ****************
		// ** Properties **
		// ****************

		Database &Cache::getDatabase ()
		{
			return db;
		}

		QDate Cache::getTodayDate ()
		{
			synchronizedReturn (dataMutex, todayDate);
		}

		QDate Cache::getOtherDate ()
		{
			synchronizedReturn (dataMutex, otherDate);
		}


		// ******************
		// ** Object lists **
		// ******************

		EntityList<Plane> Cache::getPlanes ()
		{
			synchronizedReturn (dataMutex, planes);
		}

		EntityList<Person> Cache::getPeople ()
		{
			synchronizedReturn (dataMutex, people);
		}

		EntityList<LaunchMethod> Cache::getLaunchMethods ()
		{
			synchronizedReturn (dataMutex, launchMethods);
		}

		EntityList<Flight> Cache::getFlightsToday ()
		{
			synchronizedReturn (dataMutex, flightsToday.getList ());
		}

		EntityList<Flight> Cache::getFlightsOther ()
		{
			synchronizedReturn (dataMutex, flightsOther.getList ());
		}

		EntityList<Flight> Cache::getPreparedFlights ()
		{
			synchronizedReturn (dataMutex, preparedFlights.getList ());
		}

		/**
		 * Makes a copy of the object list
		 *
		 * We don't return a (const) reference to the list because the list
		 * itself is not thread safe and accesses to the list have to
		 * synchronized. Copying the list is fast if the list is not modified
		 * thanks to Qt implicit sharing.
		 */
		template<class T> EntityList<T> Cache::getObjects () const
		{
			synchronizedReturn (dataMutex, *objectList<T> ());
		}

		// Specialize list getters (const)
		template<> const EntityList<Plane       > *Cache::objectList<Plane       > () const { return &planes       ; }
		template<> const EntityList<Person      > *Cache::objectList<Person      > () const { return &people       ; }
		template<> const EntityList<LaunchMethod> *Cache::objectList<LaunchMethod> () const { return &launchMethods; }

		// Specialize list getters (non-const)
		template<> EntityList<Plane       > *Cache::objectList<Plane       > () { return &planes       ; }
		template<> EntityList<Person      > *Cache::objectList<Person      > () { return &people       ; }
		template<> EntityList<LaunchMethod> *Cache::objectList<LaunchMethod> () { return &launchMethods; }



		// ************************
		// ** Individual objects **
		// ************************

		/**
		 * Gets an object from the cache; the database is not accessed.
		 *
		 * @param id the ID of the object
		 * @return a copy of the object
		 * @throw NotFoundException if the object is not found or id is invalid
		 */
		template<class T> T Cache::getObject (dbId id)
		{
			if (idInvalid (id)) throw NotFoundException (id);

			// TODO use a hash/map
			synchronized (dataMutex)
				foreach (const T &object, objectList<T> ()->getList ())
					if (object.getId ()==id)
						return T (object);

			throw NotFoundException (id);
		}

		// Different specialization for Flight
		template<> Flight Cache::getObject (dbId id)
		{
			// TODO use map
			synchronized (dataMutex)
			{
				if (todayDate.isValid ())
					foreach (const Flight &flight, flightsToday.getList ())
						if (flight.getId ()==id)
							return Flight (flight);

				if (otherDate.isValid ())
					foreach (const Flight &flight, flightsOther.getList ())
						if (flight.getId ()==id)
								return Flight (flight);

				foreach (const Flight &flight, preparedFlights.getList ())
					if (flight.getId ()==id)
						return Flight (flight);
			}

			throw NotFoundException (id);
		}

		/**
		 * Gets an object from the cache; the database is not accessed.
		 *
		 * @param id the ID of the object
		 * @return a newly allocated copy of the object (the caller takes
		 *         ownership) or NULL the object is not found or id is invalid
		 */
		template<class T> T* Cache::getNewObject (dbId id)
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

		/**
		 * Determines if an object exists in the cache; the database is not
		 * accessed.
		 *
		 * @param id the ID of the object
		 * @return true if the object exists, or false if not
		 */
		template<class T> bool Cache::objectExists (dbId id)
		{
			// TODO use hash/map
			synchronized (dataMutex)
				foreach (const T &object, *objectList<T> ())
					if (object.getId ()==id)
						return true;

			return false;
		}

		// TODO template
		QList<Person> Cache::getPeople (const QList<dbId> &ids)
		{
			QList<Person> result;

			foreach (dbId id, ids)
				result.append (getObject<Person> (id));

			return result;
		}

		// *** Object lookup
		dbId Cache::getPlaneIdByRegistration (const QString &registration)
		{
			// TODO use hash/map
			synchronized (dataMutex)
				foreach (const Plane &plane, planes.getList ())
					if (plane.registration.toLower ()==registration.toLower ())
						return plane.getId ();

			return invalidId;
		}

		QList<dbId> Cache::getPersonIdsByName (const QString &firstName, const QString &lastName)
		{
			// TODO use hash/map
			QList<dbId> result;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					if (person.firstName.toLower ()==firstName.toLower () && person.lastName.toLower ()==lastName.toLower ())
						result.append (person.getId ());

			return result;
		}

		/**
		 * Returns the ID of the person with the given first and last name
		 * (case insensitively) if there is exactly one such person, or an
		 * invalid id if threre are multiple or no such people
		 *
		 * @param firstName the first name of the person (the case is ignored)
		 * @param lastName the last name of the person (the case is ignored)
		 * @return the ID of the person, or an invalid ID
		 */
		dbId Cache::getUniquePersonIdByName (const QString &firstName, const QString &lastName)
		{
			// TODO use hash/map
			// TODO: instead of getting all matching people, we could stop on the first
			// duplicate.
			const QList<dbId> personIds=getPersonIdsByName (firstName, lastName);

			if (personIds.size ()==1)
				return personIds.at (0);
			else
				return invalidId;
		}

		QList<dbId> Cache::getPersonIdsByLastName (const QString &lastName)
		{
			// TODO use hash/map
			QList<dbId> result;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					if (person.lastName.toLower ()==lastName.toLower ())
						result.append (person.getId ());

			return result;
		}

		QList<dbId> Cache::getPersonIdsByFirstName (const QString &firstName)
		{
			// TODO use hash/map
			QList<dbId> result;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					if (person.firstName.toLower ()==firstName.toLower ())
						result.append (person.getId ());

			return result;
		}

		dbId Cache::getLaunchMethodByType (LaunchMethod::Type type)
		{
			// TODO use hash/map
			synchronized (dataMutex)
				foreach (const LaunchMethod &launchMethod, launchMethods.getList ())
					if (launchMethod.type==type)
						return launchMethod.getId ();

			return invalidId;
		}


		// *****************
		// ** Object data **
		// *****************

		QStringList Cache::getPlaneRegistrations ()
		{
			// TODO cache
			QStringList result;

			synchronized (dataMutex)
				foreach (const Plane &plane, planes.getList ())
					result.append (plane.registration);

			result.sort ();
			return result;
		}

		QStringList Cache::getPersonFirstNames ()
		{
			// TODO cache
			// TODO case insensitive unique
			QSet<QString> firstNames;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					firstNames.insert (person.firstName);

			// TODO sort case insensitively
			QStringList result=QStringList::fromSet (firstNames);
			result.sort ();
			return result;
		}

		QStringList Cache::getPersonFirstNames (const QString &lastName)
		{
			// TODO cache
			// TODO case insensitive unique
			QSet<QString> firstNames;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					if (person.lastName.toLower ()==lastName.toLower ())
						firstNames.insert (person.firstName);

			// TODO sort case insensitively
			QStringList result=QStringList::fromSet (firstNames);
			result.sort ();
			return result;
		}

		QStringList Cache::getPersonLastNames ()
		{
			// TODO cache
			// TODO case insensitive unique
			QSet<QString> lastNames;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					lastNames.insert (person.lastName);

			// TODO sort case insensitively
			QStringList result=QStringList::fromSet (lastNames);
			result.sort ();
			return result;
		}

		QStringList Cache::getPersonLastNames (const QString &firstName)
		{
			// TODO cache
			// TODO case insensitive unique
			QSet<QString> lastNames;

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					if (person.firstName.toLower ()==firstName.toLower ())
						lastNames.insert (person.lastName);

			// TODO sort case insensitively
			QStringList result=QStringList::fromSet (lastNames);
			result.sort ();
			return result;
		}

		QStringList Cache::getLocations ()
		{
			synchronizedReturn (dataMutex, locations);
		}

		QStringList Cache::getAccountingNotes ()
		{
			synchronizedReturn (dataMutex, accountingNotes);
		}

		QStringList Cache::getPlaneTypes ()
		{
			QSet<QString> types;

			synchronized (dataMutex)
				foreach (const Plane &plane, planes.getList ())
					types.insert (plane.type);

			// TODO sort case insensitively
			QStringList result=QStringList::fromSet (types);
			result.sort ();
			return result;

			// TODO cache
			//synchronizedReturn (dataMutex, planeTypes);
		}

		QStringList Cache::getClubs ()
		{
			QSet<QString> clubs;

			synchronized (dataMutex)
				foreach (const Plane &plane, planes.getList ())
					clubs.insert (plane.club);

			synchronized (dataMutex)
				foreach (const Person &person, people.getList ())
					clubs.insert (person.club);

			// TODO sort case insensitively
			QStringList result=QStringList::fromSet (clubs);
			result.sort ();
			return result;

			// TODO cache
			//synchronizedReturn (dataMutex, clubs);
		}


		dbId Cache::planeFlying (dbId id)
		{
			synchronized (dataMutex)
			{
				// Only use the flights of today
				foreach (const Flight &flight, flightsToday.getList ())
				{
					if (
						(flight.isFlying         () && flight.planeId==id) ||
						(flight.isTowplaneFlying () && flight.towplaneId==id))
						return flight.getId ();
				}
			}

			return invalidId;
		}

		dbId Cache::personFlying (dbId id)
		{
			synchronized (dataMutex)
			{
				// Only use the flights of today
				foreach (const Flight &flight, flightsToday.getList ())
				{
					if (
						(flight.isFlying         () && flight.pilotId    ==id) ||
						(flight.isFlying         () && flight.copilotId  ==id) ||
						(flight.isTowplaneFlying () && flight.towpilotId ==id))
						return flight.getId ();
				}
			}

			return invalidId;
		}


		// *************
		// ** Reading **
		// *************

		// TODO allow canceling (old OperationMonitor)
		int Cache::refreshPlanes (OperationMonitorInterface monitor)
		{
			monitor.status ("Flugzeuge abrufen");

			QList<Plane> newPlanes=db.getObjects<Plane> ();
			synchronized (dataMutex) planes=newPlanes;
			// TODO rebuild hashes
			// TODO rebuild planeTypes, clubs

			return 0;
		}

		int Cache::refreshPeople (OperationMonitorInterface monitor)
		{
			monitor.status ("Personen abrufen");

			QList<Person> newPeople=db.getObjects<Person> ();
			synchronized (dataMutex) people=newPeople;
			// TODO rebuild hashes
			// TODO rebuild clubs

			return 0;
		}

		int Cache::refreshLaunchMethods (OperationMonitorInterface monitor)
		{
			monitor.status ("Startarten abrufen");

			QList<LaunchMethod> newLaunchMethods=db.getObjects<LaunchMethod> ();
			synchronized (dataMutex) launchMethods=newLaunchMethods;
			// TODO rebuild hashes

			return 0;
		}

		int Cache::refreshFlights ()
		{
			refreshFlightsToday ();
			refreshFlightsOther ();
			refreshPreparedFlights ();

			return 0;
		}

		int Cache::refreshFlightsToday (OperationMonitorInterface monitor)
		{
			monitor.status ("Flüge von heute abrufen");

			QDate today=QDate::currentDate ();

			QList<Flight> newFlights=db.getFlightsDate (today);

			synchronized (dataMutex)
			{
				todayDate=today;
				flightsToday.replaceList (newFlights);
			}

			return 0;
		}

		int Cache::refreshFlightsOther (OperationMonitorInterface monitor)
		{
			if (otherDate.isNull ()) return 0;

			monitor.status (trUtf8 ("Flüge von %1 abrufen").arg (otherDate.toString (Qt::LocaleDate)));

			QList<Flight> newFlights=db.getFlightsDate (otherDate);
			synchronized (dataMutex) flightsOther.replaceList (newFlights);

			return 0;
		}

		int Cache::fetchFlightsOther (QDate date, OperationMonitorInterface monitor)
		{
			monitor.status (trUtf8 ("Flüge für %1 werden abgerufen").arg (date.toString (Qt::LocaleDate)));

			if (date.isNull ())
				return 0;

			QList<Flight> newFlights=db.getFlightsDate (date);

			synchronized (dataMutex)
			{
				otherDate=date;
				flightsOther.replaceList (newFlights);
			}

			return 0;
		}

		int Cache::refreshPreparedFlights (OperationMonitorInterface monitor)
		{
			monitor.status (trUtf8 ("Vorbereitete Flüge abrufen"));

			QList<Flight> newFlights=db.getPreparedFlights ();
			synchronized (dataMutex) preparedFlights.replaceList (newFlights);

			return 0;
		}

		int Cache::refreshLocations (OperationMonitorInterface monitor)
		{
			monitor.status (trUtf8 ("Flugplätze abrufen"));

			QStringList newLocations=db.listLocations ();
			synchronized (dataMutex) locations=newLocations;

			return 0;
		}

		int Cache::refreshAccountingNotes (OperationMonitorInterface monitor)
		{
			monitor.status (trUtf8 ("Abrechnungshinweise abrufen"));

			QStringList newAccountingNotes=db.listAccountingNotes ();
			synchronized (dataMutex) accountingNotes=newAccountingNotes;

			return 0;
		}

		bool Cache::refreshAll (OperationMonitorInterface monitor)
		{
			// Refresh planes and people before refreshing flights!
			monitor.progress (0, 8); refreshPlanes          (monitor);
			monitor.progress (1, 8); refreshPeople          (monitor);
			monitor.progress (2, 8); refreshLaunchMethods   (monitor);
			monitor.progress (3, 8); refreshFlightsToday    (monitor);
			monitor.progress (4, 8); refreshFlightsOther    (monitor);
			monitor.progress (5, 8); refreshPreparedFlights (monitor);
			monitor.progress (6, 8); refreshLocations       (monitor);
			monitor.progress (7, 8); refreshAccountingNotes (monitor);
			monitor.progress (8, 8, "Fertig");
			return true;
		}


		// *********************
		// ** Change handling **
		// *********************

		// This template is specialized for T==Flight
		template<class T> void Cache::objectAdded (const T &object)
		{
			// Add the object to the cache
			synchronized (dataMutex) objectList<T> ()->append (object);
			// TODO update hashes
		}

		template<> void Cache::objectAdded<Flight> (const Flight &flight)
		{
			synchronized (dataMutex)
			{
				if (flight.isPrepared ())
					preparedFlights.append (flight);
				else if (flight.effdatum ()==todayDate)
					flightsToday.append (flight);
				else if (flight.effdatum ()==otherDate)
					// If otherDate is the same as today, this is not reached.
					flightsOther.append (flight);
				//else
				//	we're not interested in this flight
			}
		}

		// This template is specialized for T==Flight
		template<class T> void Cache::objectDeleted (dbId id)
		{
			// Remove the object from the cache
			synchronized (dataMutex) objectList<T> ()->removeById (id);
		}

		template<> void Cache::objectDeleted<Flight> (dbId id)
		{
			// If any of the lists contain this flight, remove it
			preparedFlights.removeById (id);
			flightsToday.removeById (id);
			flightsOther.removeById (id);
		}

		// This template is specialized for T==Flight
		template<class T> void Cache::objectUpdated (const T &object)
		{
			// TODO if the object is not in the cache, add it and log an error
			// TODO use EntityList methods

				// Update the cache
			synchronized (dataMutex)
				objectList<T> ()->replaceById (object.getId (), object);
		}

		template<> void Cache::objectUpdated<Flight> (const Flight &flight)
		{
			// If the date or the prepared status of a flight changed, we may have to
			// relocate it to a different list. If the date is changed, it may not be
			// on any list at all any more; or it may not have been on any list before
			// (although the UI does not provide a way to modify a flight that is not
			// on one of these lists, but something like that may well be added, and
			// even if not, we'd still have to handle this case).

			// Determine which list the flight should be in (or none). Replace it if
			// it already exists, add it if not, and remove it from the other lists.

			synchronized (dataMutex)
			{
				if (flight.isPrepared ())
				{
					preparedFlights.replaceOrAdd (flight.getId (), flight);
					flightsToday.removeById (flight.getId ());
					flightsOther.removeById (flight.getId ());
				}
				else if (flight.effdatum ()==todayDate)
				{
					preparedFlights.removeById (flight.getId ());
					flightsToday.replaceOrAdd (flight.getId (), flight);
					flightsOther.removeById (flight.getId ());
				}
				else if (flight.effdatum ()==otherDate)
				{
					// If otherDate is the same as today, this is not reached.
					preparedFlights.removeById (flight.getId ());
					flightsToday.removeById (flight.getId ());
					flightsOther.replaceOrAdd (flight.getId (), flight);
				}
				else
				{
					// The flight should not be on any list - remove it from all lists
					preparedFlights.removeById (flight.getId ());
					flightsToday.removeById (flight.getId ());
					flightsOther.removeById (flight.getId ());
				}
			}
		}

		void Cache::dbChanged (Event::DbEvent event)
		{
			std::cout << "Cache: "<< event.toString () << std::endl;

			// factorize this method: processDbEvent<T> (event)

			// This is ugly, but we can't pass a template class instance as a
			// signal parameter
			switch (event.getType ())
			{
				case Event::DbEvent::typeAdd:
					switch (event.getTable ())
					{
						case Event::DbEvent::tableFlights      : objectAdded (event.getValue<Flight>       ()); break;
						case Event::DbEvent::tableLaunchMethods: objectAdded (event.getValue<LaunchMethod> ()); break;
						case Event::DbEvent::tablePeople       : objectAdded (event.getValue<Person>       ()); break;
						case Event::DbEvent::tablePlanes       : objectAdded (event.getValue<Plane>        ()); break;
					}
					break;
				case Event::DbEvent::typeChange:
					switch (event.getTable ())
					{
						case Event::DbEvent::tableFlights      : objectUpdated (event.getValue<Flight      > ()); break;
						case Event::DbEvent::tableLaunchMethods: objectUpdated (event.getValue<LaunchMethod> ()); break;
						case Event::DbEvent::tablePeople       : objectUpdated (event.getValue<Person      > ()); break;
						case Event::DbEvent::tablePlanes       : objectUpdated (event.getValue<Plane       > ()); break;
					}
					break;
				case Event::DbEvent::typeDelete:
					switch (event.getTable ())
					{
						case Event::DbEvent::tableFlights      : objectDeleted<Flight      > (event.getId ()); break;
						case Event::DbEvent::tableLaunchMethods: objectDeleted<LaunchMethod> (event.getId ()); break;
						case Event::DbEvent::tablePeople       : objectDeleted<Person      > (event.getId ()); break;
						case Event::DbEvent::tablePlanes       : objectDeleted<Plane       > (event.getId ()); break;
					}
					break;
				// no default
			}

			// Re-emit the event
			std::cout << "Cache reemitting " << event.toString() << std::endl;

			emit changed (event);
		}


		// ****************************
		// ** Template instantiation **
		// ****************************

		template EntityList<Plane       > Cache::getObjects () const;
		template EntityList<Person      > Cache::getObjects () const;
		template EntityList<LaunchMethod> Cache::getObjects () const;


		// Instantiate the get method templates
		template Flight       Cache::getObject (dbId id);
		template Plane        Cache::getObject (dbId id);
		template Person       Cache::getObject (dbId id);
		template LaunchMethod Cache::getObject (dbId id);

		template Flight       *Cache::getNewObject (dbId id);
		template Plane        *Cache::getNewObject (dbId id);
		template Person       *Cache::getNewObject (dbId id);
		template LaunchMethod *Cache::getNewObject (dbId id);


		// Instantiate the change method templates (not for Flight - specialized)
		template void Cache::objectAdded<Plane       > (const Plane        &object);
		template void Cache::objectAdded<Person      > (const Person       &object);
		template void Cache::objectAdded<LaunchMethod> (const LaunchMethod &object);

		template void Cache::objectDeleted<Plane       > (dbId id);
		template void Cache::objectDeleted<Person      > (dbId id);
		template void Cache::objectDeleted<LaunchMethod> (dbId id);

		template void Cache::objectUpdated<Plane       > (const Plane        &plane );
		template void Cache::objectUpdated<Person      > (const Person       &flight);
		template void Cache::objectUpdated<LaunchMethod> (const LaunchMethod &flight);
	}
}
