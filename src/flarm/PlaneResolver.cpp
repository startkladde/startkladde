#include "PlaneResolver.h"

#include <iostream>

#include "src/db/cache/Cache.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/i18n/notr.h"
#include "src/config/Settings.h"

PlaneResolver::PlaneResolver (Cache &cache):
	cache (cache)
{
}

PlaneResolver::~PlaneResolver ()
{
}

PlaneResolver::Result PlaneResolver::resolvePlaneByFlarmId (const QString &flarmId)
{
	dbId planeId=cache.getPlaneIdByFlarmId (flarmId);
	if (idValid (planeId))
		return Result (planeId, Maybe<FlarmNetRecord>::invalid ());

	return Result::invalid ();
}

PlaneResolver::Result PlaneResolver::resolvePlaneByFlarmNetDatabase (const QString &flarmId)
{
	if (!Settings::instance ().flarmNetEnabled)
		return Result::invalid ();

	try
	{
		// Try to look up the plane via FlarmNet: flarmId => registration => id
		dbId flarmNetRecordId = cache.getFlarmNetRecordIdByFlarmId (flarmId);
		if (!idValid (flarmNetRecordId))
			// No larmNet record with that Flarm ID
			return Result::invalid ();

		FlarmNetRecord flarmNetRecord = cache.getObject<FlarmNetRecord> (flarmNetRecordId);
		QString registration = flarmNetRecord.registration;

		if (registration.isEmpty ())
			return Result::invalid ();

		dbId planeId=cache.getPlaneIdByRegistration (registration);
		if (idValid (planeId))
		{
			return Result (planeId, flarmNetRecord);
		}
		else
		{
			return Result (invalidId, flarmNetRecord);
		}
	}
	catch (Cache::NotFoundException &)
	{
		// This should not happen because we only fetch objects for IDs we
		// retrieved from the cache, but you never know...
		return Result::invalid ();
	}

	return Result::invalid ();
}

PlaneResolver::Result PlaneResolver::resolvePlane (const QString &flarmId)
{
	Result result=Result::invalid ();

	// Try to find the plane by Flarm ID
	result=resolvePlaneByFlarmId (flarmId);
	if (result.planeFound ())
		return result;

	// Try to find the plane via FlarmNet database
	result=resolvePlaneByFlarmNetDatabase (flarmId);
	if (result.planeFound ())
		return result;
	else if (result.flarmNetRecordFound ())
		return result;

	// None of the criteria matched, and no useful data was returned.
	return Result::invalid ();
}
