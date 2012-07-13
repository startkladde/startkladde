#ifndef FLIGHTRESOLVER_H_
#define FLIGHTRESOLVER_H_

#include <QString>

#include "src/db/dbId.h"

class Cache;
class Flight;

/**
 * Provides methods for looking up a flight for a given Flarm ID
 *
 * Given a Flarm ID, a flight can be identified by different criteria, in
 * decreasing order of reliability:
 *   1. The Flarm ID matches the Flarm ID of the flight.
 *      This means that the flight was created automatically. This is the most
 *      reliable criterion as it does not rely on any user-entered data. This
 *      criterion can only fail if the Flarm ID changes during the flight.
 *   2. The Flarm ID matches the Flarm ID of the plane of the flight. This is
 *      the typical case for prepared flights of known planes.
 *   3. The Flarm ID matches the Flarm ID of a FlarmNet record whose
 *      registration matches the registration of the plane of the flight.
 *      This is the least reliable criterion because the FlarmNet database
 *      itself may be inaccurate, and our copy of the FlarmNet database may be
 *      outdated.
 *
 * Note that FlarmNet lookup is currently not implemented.
 *
 * See also: the wiki page "Flarm".
 */
class FlightResolver
{
	public:
		class Result
		{
			public:
				dbId flightId;
				dbId planeId;
				// FIXME: we get the complet plane data from FlarmNet, so there
				// shold probably be a FlarmNetRecord here instead of the
				// registration.
				QString registration;

				Result (dbId flightId, dbId planeId, const QString &registration):
					flightId (flightId), planeId (planeId), registration (registration)
				{
				}

				static Result invalid ()
				{
					return Result (invalidId, invalidId, QString ());
				}

				bool isValid           () const { return idValid (flightId); }
				bool planeFound        () const { return idValid (planeId);  }
				bool registrationFound () const { return !registration.isEmpty (); }
		};

		FlightResolver (Cache &cache);
		virtual ~FlightResolver ();

		Result resolveFlight                   (const QList<Flight> &flights, const QString &flarmId);

	protected:
		Result resolveFlightByFlarmId          (const QList<Flight> &flights, const QString &flarmId);
		Result resolveFlightByPlaneFlarmId     (const QList<Flight> &flights, const QString &flarmId);
		Result resolveFlightByFlarmNetDatabase (const QList<Flight> &flights, const QString &flarmId);

	private:
		Cache &cache;
};

#endif
