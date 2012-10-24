/*
 * FlarmHandling.cpp
 *
 *  Created on: 24.10.2012
 *      Author: martin
 */

#include "src/flarm/FlarmHandling.h"

#include <QWidget>

#include "src/gui/dialogs.h"
#include "src/db/DbManager.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/flarm/PlaneResolver.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/gui/windows/objectEditor/PlaneEditorPane.h"

FlarmHandling::FlarmHandling ()
{
}

FlarmHandling::~FlarmHandling ()
{
}

dbId FlarmHandling::interactiveIdentifyPlane (QWidget *parent, DbManager &dbManager, dbId flightId)
{
	Cache &cache=dbManager.getCache ();

	try
	{
		Flight flight=cache.getObject<Flight> (flightId);

		// We can only do this for automatically created flights
		if (flight.getFlarmId ().isEmpty ())
		{
			QMessageBox::information (parent, qApp->translate ("FlarmHandling", "Identify plane"),
				qApp->translate ("FlarmHandling", "The plane cannot be "
					"identified because this flight was not created automatically."));
			return invalidId;
		}

		// Lookup the plane
		PlaneResolver::Result result=PlaneResolver (cache).resolvePlane (flight.getFlarmId ());

		// Let's see what we've got...
		if (result.planeFound ())
		{
			Plane plane=cache.getObject<Plane> (result.planeId);
			// Offer the user to use this plane
			QString title=qApp->translate ("FlarmHandling", "Use plane?");
			QString text =qApp->translate ("FlarmHandling", "The plane seems "
				"to be a %1 with registration %2. Do you want to use this plane?")
				.arg (plane.type).arg (plane.fullRegistration ());
			// FIXME no type?
			// FIXME allow multiple
			// FIXME Flarm ID mismatch

			if (yesNoQuestion (parent, title, text))
				return result.planeId;
		}
		else if (result.flarmNetRecordFound ())
		{
			// Offer the user to create a plane with the FlarmNet data
			// FIXME no type
			// FIXME we need the options: create plane with FlarmNet data, create plane from scratch, cancel
			QString title=qApp->translate ("FlarmHandling", "Automatically create plane?");
			QString text =qApp->translate ("FlarmHandling", "The plane was not "
				"found in the database. However, the FlarmNet database indicates "
				"that the plane might be a %1 with registration %2. Do you want "
				"to create this plane?")
				.arg (result.flarmNetRecord->type)
				.arg (result.flarmNetRecord->fullRegistration ());

			if (yesNoQuestion (parent, title, text))
			{
				PlaneEditorPaneData paneData;
				paneData.flarmIdReadOnly=true;
				return ObjectEditorWindow<Plane>::createObjectPreset (parent,
					dbManager, result.flarmNetRecord->toPlane (), &paneData, NULL);
			}
		}
		else
		{
			// Offer the user to create a plane from scratch
			QString title=qApp->translate ("FlarmHandling", "Create plane?");
			QString text =qApp->translate ("FlarmHandling", "The plane was not found. Do you want to create it?");

			if (yesNoQuestion (parent, title, text))
			{
				PlaneEditorPaneData paneData;
				paneData.flarmIdReadOnly=true;
				paneData.flarmId=flight.getFlarmId ();
				return ObjectEditorWindow<Plane>::createObject (parent,
					dbManager, &paneData);
			}
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		return invalidId;
	}

	return invalidId;
}
