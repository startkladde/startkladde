#include "Cache.h"

/*
 * Currently, the update methods call the removed and added methods. This may
 * or may not be optimal or even correct.
 *
 * Also, we don't remove entries that may still be valid, for example name
 * parts where there may be another person with the same name part. For the
 * SortedSets, this could be implemented by making the sets count the number of
 * times a value has been added; for the MultiHashes it would be a bit more
 * work.
 */

#include "src/concurrent/synchronized.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/text.h"

// All of these methods do not handle the by-id hashes
// These methods must be specialized before they are used.
// No default that does nothing to avoid forgetting one
// The individual clear methods do not clear hashes use for multiple
// object types (such as clubs which is used for people and planes)

// ********************
// ** Multiple types **
// ********************

void Cache::clearMultiTypeHashes ()
{
	synchronized (dataMutex)
	{
		clubs.clear ();
	}
}


// ************
// ** Planes **
// ************

template<> void Cache::clearHashes<Plane> ()
{
	synchronized (dataMutex)
	{
		planeTypes.clear ();
		planeRegistrations.clear ();
		planeIdsByRegistration.clear ();
		// clubs is used by multiple types
	}
}

template<> void Cache::updateHashesObjectAdded<Plane> (const Plane &plane)
{
	synchronized (dataMutex)
	{
		if (!blank (plane.type)) planeTypes.insert (plane.type);
		planeRegistrations.insert (plane.registration);
		planeIdsByRegistration.insert (plane.registration.toLower (), plane.getId ());
		if (!blank (plane.club)) clubs.insert (plane.club);
	}
}

template<> void Cache::updateHashesObjectDeleted<Plane> (dbId id, const Plane *oldPlane)
{
	synchronized (dataMutex)
	{
		// Leave planeTypes
		// Leave planeRegistrations - it may have been a duplicate
		if (oldPlane) planeIdsByRegistration.remove (oldPlane->registration.toLower (), id);
		// Leave clubs
	}
}

template<> void Cache::updateHashesObjectUpdated<Plane> (const Plane &plane, const Plane *oldPlane)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<Plane> (plane.getId (), oldPlane);
		updateHashesObjectAdded (plane);
	}
}


// ************
// ** People **
// ************


template<> void Cache::clearHashes<Person> ()
{
	synchronized (dataMutex)
	{
		personLastNames.clear ();
		personFirstNames.clear ();
		lastNamesByFirstName.clear ();
		firstNamesByLastName.clear ();
		personIdsByLastName.clear ();
		personIdsByFirstName.clear ();
		personIdsByName.clear ();
		// clubs is used by multiple types
	}
}

template<> void Cache::updateHashesObjectAdded<Person> (const Person &person)
{
	synchronized (dataMutex)
	{
		const QString &last =person.lastName ; QString lastLower =last .toLower ();
		const QString &first=person.firstName; QString firstLower=first.toLower ();
		dbId id=person.getId ();

		personLastNames.insert (last);
		personFirstNames.insert (first);
		lastNamesByFirstName.insert (firstLower, last );
		firstNamesByLastName.insert (lastLower , first);
		personIdsByLastName .insert (lastLower , id);
		personIdsByFirstName.insert (firstLower, id);
		personIdsByName.insert (QPair<QString, QString> (lastLower, firstLower), id);
		if (!blank (person.club)) clubs.insert (person.club);
	}
}

template<> void Cache::updateHashesObjectDeleted<Person> (dbId id, const Person *oldPerson)
{
	synchronized (dataMutex)
	{
		// Leave personLastNames
		// Leave personFirstNames
		// Leave lastNamesByFirstName
		// Leave firstNamesByLastName
		if (oldPerson)
		{
			QString lastLower =oldPerson-> lastName.toLower ();
			QString firstLower=oldPerson->firstName.toLower ();

			if (oldPerson) personIdsByLastName.remove (lastLower, id);
			if (oldPerson) personIdsByFirstName.remove (firstLower, id);
			if (oldPerson) personIdsByName.remove (QPair<QString, QString> (lastLower, firstLower), id);
		}
		// Leave clubs
	}
}

template<> void Cache::updateHashesObjectUpdated<Person> (const Person &person, const Person *oldPerson)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<Person> (person.getId (), oldPerson);
		updateHashesObjectAdded (person);
	}
}


// ********************
// ** Launch methods **
// ********************

template<> void Cache::clearHashes<LaunchMethod> ()
{
	synchronized (dataMutex)
	{
		launchMethodIdsByType.clear ();
	}
}

template<> void Cache::updateHashesObjectAdded<LaunchMethod> (const LaunchMethod &launchMethod)
{
	synchronized (dataMutex)
	{
		launchMethodIdsByType.insert (launchMethod.type, launchMethod.getId ());
	}
}

template<> void Cache::updateHashesObjectDeleted<LaunchMethod> (dbId id, const LaunchMethod *oldLaunchMethod)
{
	synchronized (dataMutex)
	{
		if (oldLaunchMethod) launchMethodIdsByType.remove (oldLaunchMethod->type, id);
	}
}

template<> void Cache::updateHashesObjectUpdated<LaunchMethod> (const LaunchMethod &launchMethod, const LaunchMethod *oldLaunchMethod)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<LaunchMethod> (launchMethod.getId (), oldLaunchMethod);
		updateHashesObjectAdded (launchMethod);
	}
}


// *************
// ** Flights **
// *************

template<> void Cache::clearHashes<Flight> ()
{
	synchronized (dataMutex)
	{
		locations.clear ();
		accountingNotes.clear ();
	}
}

template<> void Cache::updateHashesObjectAdded<Flight> (const Flight &flight)
{
	synchronized (dataMutex)
	{
		if (!blank (flight.departureLocation)) locations.insert (flight.departureLocation);
		if (!blank (flight.landingLocation)) locations.insert (flight.landingLocation);
		if (!blank (flight.accountingNotes)) accountingNotes.insert (flight.accountingNotes);
	}
}

template<> void Cache::updateHashesObjectDeleted<Flight> (dbId id, const Flight *oldFlight)
{
	synchronized (dataMutex)
	{
		(void)id;
		(void)oldFlight;
		// Leave locations (must include values from all flights)
		// Leave accountingNotes (must include values from all flights)
	}
}

template<> void Cache::updateHashesObjectUpdated<Flight> (const Flight &flight, const Flight *oldFlight)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<Flight> (flight.getId (), oldFlight);
		updateHashesObjectAdded (flight);
	}
}
