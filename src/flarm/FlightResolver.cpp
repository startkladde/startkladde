#include <QtCore/QDebug>

#include <src/flarm/FlightResolver.h>
#include "src/db/cache/Cache.h"
#include "src/model/Flight.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/i18n/notr.h"

FlightResolver::FlightResolver (Cache &cache):
	cache (cache)
{
}

FlightResolver::~FlightResolver ()
{
}

/**
 * Finds a flight whose Flarm ID matches the given Flarm ID
 *
 * The result can contain:
 *   - a flight ID if a flight was found
 *   - nothing if no flight was found
 *
 * @param flights the flights to search
 * @param flarmId the Flarm ID to search for
 * @return the search result. If the flight was found, the flight ID is set. The
 *         plane ID and registration are never set.
 */
FlightResolver::Result FlightResolver::resolveFlightByFlarmId (const QList<Flight> &flights, const QString &flarmId)
{
	foreach (const Flight &flight, flights)
		if (flight.getFlarmId ()==flarmId)
			return Result (flight.getId (), invalidId, QString ());

	return Result::invalid ();
}

/**
 * Finds a flight with a plane whose Flarm ID matches the given Flarm ID
 *
 * First, we look for a plane with the given Flarm ID. If the plane is found,
 * we look for a flight with that plane.
 *
 * The result can contain:
 *   - a plane ID and a flight ID if a plane and a flight were found
 *   - only a plane ID if a plane was found, but there is no flight with that
 *     plane
 *   - nothing if no plane was found
 *
 * @param flights the flights to search
 * @param flarmId the Flarm ID to search for
 * @return the search result. If the plane was found, the plane ID is set. If
 *         the flight was found, the flight ID is set. The registration is never
 *         set.
 */
FlightResolver::Result FlightResolver::resolveFlightByPlaneFlarmId (const QList<Flight> &flights, const QString &flarmId)
{
	// Try to find a flight with a plane with matching Flarm ID
	// FIXME what if there are multiple planes with this Flarm ID?
	dbId planeId=cache.getPlaneIdByFlarmId (flarmId);
	if (idValid (planeId))
	{
		// We found the plane in the database. See if any of the flights is using this plane.
		foreach (const Flight &flight, flights)
			if (flight.getPlaneId ()==planeId)
				return Result (flight.getId (), planeId, QString ());

		return Result (invalidId, planeId, QString ());
	}
	else
	{
		return Result::invalid ();
	}
}

/**
 * Finds a flight with a plane whose registration matches the registration of a
 * FlarmNet database entry whose Flarm ID matches the given Flarm ID
 *
 * First, we look for a FlarmNet entry with the given Flarm ID. If the FlarmNet
 * entry is found, we look for a plane with the registration from that FlarmNet
 * entry. If a plane is found, we look for a flight with that plane.
 *
 * The result can contain:
 *   - a registration, plane ID and flight ID if a matching FlarmNet entry, a
 *     matching plane and a matching flight were found
 *   - a registration and plane ID if a matching FlarmNet entry and a matching
 *     plane were found, but there is not flight with that plane
 *   - just a registration if a matching FlarmNet entry was found, but there
 *     is no plane with that registration
 *   - nothing if no matching FlarmNet entry was found
 *
 * @param flights the flights to search
 * @param flarmId the Flarm ID to search for
 * @return the search result. If a FlarmNet database entry was found, the
 *         registration is set. If the plane was found, the plane ID is set. If
 *         the flight was found, the flight ID is set.
 */
FlightResolver::Result FlightResolver::resolveFlightByFlarmNetDatabase (const QList<Flight> &flights, const QString &flarmId)
{
	// Q_UNUSED(flights);
	// Try to look up the plane via FlarmNet: flarmId => registration => id
	dbId flarmNetRecordId = cache.getFlarmNetRecordIdByFlarmId (flarmId);
	FlarmNetRecord* flarmNetRecord = new FlarmNetRecord (cache.getObject<FlarmNetRecord> (flarmNetRecordId));
	QString registration = flarmNetRecord->getRegistration ();
	qDebug () << "registration: " << registration << endl; 
	delete flarmNetRecord;
	if (!registration.isEmpty ())
	{
		// We found the registration in the FlarmNet database. Try to find the plane with that registration.
		dbId planeId=cache.getPlaneIdByRegistration (registration);
		if (idValid (planeId))
		{
			// We found the plane in the database. See if any of the flights is using this plane.
			foreach (const Flight &flight, flights)
				if (flight.getPlaneId ()==planeId)
					return Result (flight.getId (), planeId, registration);

			// We identified the plane, but there is no flight using that plane.
			return Result (invalidId, planeId, registration);
		}
		else
		{
			return Result (invalidId, invalidId, registration);
		}
	}
	else
	{
		return Result::invalid ();
	}
}

/**
 * Finds a flight associated with a given Flarm ID
 *
 * The criteria are tested in order of reliability, that is, the result will be
 * the most reliable result we can find.
 *
 * @param flights the flights to search
 * @param flarmId the Flarm ID to match
 * @return the lookup result. If the flight was found, the flight ID is set.
 *         Depending on the matching criterion, the plane ID and registration
 *         may or may not be set.
 */
FlightResolver::Result FlightResolver::resolveFlight (const QList<Flight> &flights, const QString &flarmId)
{
	Result result=Result::invalid ();

	// Try to find a flight by Flarm ID (criterion 1)
	result=resolveFlightByFlarmId (flights, flarmId);
	if (result.isValid ())
	{
		// We found the flight
		std::cout << qnotr ("Flight found with Flarm ID %1: %2").arg (flarmId).arg (result.flightId) << std::endl;
		return result;
	}

	// Try to find a flight by the plane's Flarm ID (criterion 2)
	result=resolveFlightByPlaneFlarmId (flights, flarmId);
	if (result.isValid ())
	{
		// We found the flight
		std::cout << qnotr ("Flight found with plane %1: %2").arg (result.planeId).arg (result.flightId) << std::endl;
		return result;
	}
	else if (result.planeFound ())
	{
		// We identified the plane, but there is no flight using that plane.
		// There is no use in trying FlarmNet - we already know the plane.
		std::cout << qnotr ("No flight found with plane %1").arg (result.planeId) << std::endl;
		return result;
	}

	// Try to find a flight by plane via FlarmNet database (criterion 3)
	result=resolveFlightByFlarmNetDatabase (flights, flarmId);
	if (result.isValid ())
	{
		// We found the flight
		std::cout << qnotr ("Flight found with plane %1: %2").arg (result.planeId).arg (result.flightId) << std::endl;
		return result;
	}
	else if (result.planeFound ())
	{
		// We identified the plane, but there is no flight using that plane.
		std::cout << qnotr ("No flight found with plane %1").arg (result.planeId) << std::endl;
		return result;
	}
	else if (result.registrationFound ())
	{
		// We found the registration, but there is no plane with that
		// registration and therefore, we cannot identify the flight.
		std::cout << qnotr ("No flight found with plane %1").arg (result.registration) << std::endl;
		return result;
	}

	// None of the criteria matched, and no useful data was returned.
	std::cout << qnotr ("No flight found for Flarm ID %1").arg (flarmId) << std::endl;
	return Result::invalid ();
}
