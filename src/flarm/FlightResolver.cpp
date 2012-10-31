#include <src/flarm/FlightResolver.h>

#include <QtCore/QDebug>

#include "src/flarm/PlaneResolver.h"
#include "src/db/cache/Cache.h"
#include "src/model/Flight.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/i18n/notr.h"
#include "src/config/Settings.h"

FlightResolver::FlightResolver (Cache &cache):
	cache (cache)
{
}

FlightResolver::~FlightResolver ()
{
}

/**
 * Tries to find a flight with the given Flarm ID
 *
 * The result can contain:
 *
 * Flight | Plane | FNR | Description
 * -------+-------+-----+--------------------
 * yes    | no    | no  | flight was found
 * -------+-------+-----+--------------------
 * no     | no    | no  | no flight was found
 */
FlightResolver::Result FlightResolver::resolveFlightByFlarmId (const QList<Flight> &candidates, const QString &flarmId)
{
	foreach (const Flight &flight, candidates)
		if (flight.getFlarmId ()==flarmId)
			return Result (flight.getId (), NULL, NULL);

	return Result::nothing ();
}

/**
 * Tries to find a plane for the given Flarm ID (by any means), and, on success,
 * a flight with that plane.
 *
 * The result can contain:
 *
 * Flight | Plane | FNR | Description
 * -------+-------+-----+-----------------------------------------------------------
 * yes    | yes   | no  | flight found, plane found directly
 * yes    | yes   | yes | flight found, plane found via FlarmNet
 * -------+-------+-----+-----------------------------------------------------------
 * no     | yes   | no  | flight not found, but a plane was found directly
 * no     | yes   | yes | flight not found, but a plane was found via FlarmNet
 * -------+-------+-----+-----------------------------------------------------------
 * no     | no    | yes | plane and flight not found, but a FlarmNet entry was found
 * -------+-------+-----+-----------------------------------------------------------
 * no     | no    | no  | nothing was found
 */
FlightResolver::Result FlightResolver::resolveFlightByPlane (const QList<Flight> &candidates, const QString &flarmId)
{
	// Try to resolve the plane
	PlaneResolver::Result planeResult=PlaneResolver (cache).resolvePlane (flarmId);

	// Return just the FlarmNet record (if any) if there is no plane (5+6)
	if (!planeResult.plane.isValid ())
		return Result (0, NULL, planeResult.flarmNetRecord);

	// Found a plane. Try to find a flight with that plane.
	dbId planeId=planeResult.plane->getId ();
	foreach (const Flight &flight, candidates)
	{
		if (flight.getPlaneId ()==planeId)
		{
			// Found a flight. Return it along with the plane and, potentially,
			// the FlarmNet record (1+2)
			return Result (flight.getId (), planeResult.plane, planeResult.flarmNetRecord);
		}
	}

	// No flight found. Return the plane and FlarmNet record (if any) (3+4)
	return Result (invalidId, planeResult.plane, planeResult.flarmNetRecord);
}


/**
 * Tries to finds a flight for a given Flarm ID from a list of candidate
 * flights, by any means supported and enabled in the configuration
 *
 * The criteria are tested in order of reliability, that is, the result will be
 * the most reliable result we can find, even if another flight matches with a
 * lower-reliability criterion.
 *
 * The result can contain:
 *
 * Flight | Plane | FNR | Description
 * -------+-------+-----+-----------------------------------------------------------
 * yes    | no    | no  | flight found directly
 * yes    | yes   | no  | flight found via plane, plane found directly
 * yes    | yes   | yes | flight found via plane, plane found via FlarmNet
 * -------+-------+-----+-----------------------------------------------------------
 * no     | yes   | no  | flight not found, but a plane was found directly
 * no     | yes   | yes | flight not found, but a plane was found via FlarmNet
 * -------+-------+-----+-----------------------------------------------------------
 * no     | no    | yes | plane and flight not found, but a FlarmNet entry was found
 * -------+-------+-----+-----------------------------------------------------------
 * no     | no    | no  | nothing was found
 *
 * In other words, the only combination that is not possible is a Flight and a
 * FlarmNet record, but no plane.
 *
 * If the result contains a plane, it is guaranteed to have either no Flarm ID
 * or the Flarm ID we're looking for.
 */
FlightResolver::Result FlightResolver::resolveFlight (const QList<Flight> &candidates, const QString &flarmId)
{
	Result result=Result::nothing ();

	// Try to find a flight by Flarm ID
	result=resolveFlightByFlarmId (candidates, flarmId);

	// If we found a flight, return it
	if (idValid (result.flightId))
		return result;

	// Try to find a flight by plane
	result=resolveFlightByPlane (candidates, flarmId);

	// Whatever we got, we return
	return result;
}
