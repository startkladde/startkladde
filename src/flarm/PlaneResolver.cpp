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
	{
		try
		{
			Plane plane=cache.getObject<Plane> (planeId);
			return Result (plane, Maybe<>::invalid);
		}
		catch (Cache::NotFoundException &ex)
		{
		}
	}

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
			// Make sure that the plane's Flarm ID actually matches the Flarm ID
			// we're looking for - the data from the FlarmNet database may be
			// outdated.
			Plane plane=cache.getObject<Plane> (planeId);
			if (plane.flarmId==flarmId)
			{
				return Result (plane, flarmNetRecord);
			}
		}
		else
		{
			return Result (Maybe<>::invalid, flarmNetRecord);
		}
	}
	catch (Cache::NotFoundException &) {}
	{
		// Should not happen
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
