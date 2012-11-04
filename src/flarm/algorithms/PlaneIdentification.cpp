#include "PlaneIdentification.h"

#include <QWidget>

#include "src/gui/dialogs.h"
#include "src/db/DbManager.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/flarm/algorithms/PlaneLookup.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/gui/windows/objectEditor/PlaneEditorPane.h"
#include "src/text.h"

// FIXME: identify known plane - warn if the identified plane is different from
// the current one

PlaneIdentification::PlaneIdentification (DbManager &dbManager, bool messageOnFailure, QWidget *parent):
	dbManager (dbManager), messageOnFailure (messageOnFailure), parent (parent)
{
}

PlaneIdentification::~PlaneIdentification ()
{
}

bool PlaneIdentification::queryUsePlane (const Plane &plane)
{
	// Offer the user to use this plane

	QString title=qApp->translate ("PlaneIdentification", "Use plane?");

	// TODO test both cases
	QString text;
	if (isBlank (plane.type))
		text=qApp->translate ("PlaneIdentification",
			"The plane seems to be %1. "
			"Do you want to use this plane?")
			.arg (plane.fullRegistration ());
	else
		text=qApp->translate ("PlaneIdentification",
			"The plane seems to be a %1 with registration %2. "
			"Do you want to use this plane?")
			.arg (plane.type).arg (plane.fullRegistration ());

	return yesNoQuestion (parent, title, text);
}

bool PlaneIdentification::queryCreatePlane (const FlarmNetRecord &flarmNetRecord)
{
	// Offer the user to create a plane with the FlarmNet data
	QString title=qApp->translate ("PlaneIdentification", "Automatically create plane?");

	// TODO test both cases
	QString text;
	if (isBlank (flarmNetRecord.type))
		text=qApp->translate ("PlaneIdentification",
			"The plane was not found in the database. "
			"However, the FlarmNet database indicates that the plane might be "
			"a %1 with registration %2. "
			"Do you want to create this plane?")
			.arg (flarmNetRecord.type)
			.arg (flarmNetRecord.fullRegistration ());
	else
		text=qApp->translate ("PlaneIdentification",
			"The plane was not found in the database. "
			"However, the FlarmNet database indicates that the registration of "
			"the plane might be %1. "
			"Do you want to create this plane?")
			.arg (flarmNetRecord.fullRegistration ());

	return yesNoQuestion (parent, title, text);
}

void PlaneIdentification::notCreatedAutomaticallyMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane cannot be "
			"identified because this flight was not created automatically."));
}

void PlaneIdentification::identificationFailureMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane could not be identified."));
}

dbId PlaneIdentification::interactiveCreatePlane (const FlarmNetRecord &flarmNetRecord)
{
	PlaneEditorPaneData paneData;
	paneData.flarmIdReadOnly=true;

	return ObjectEditorWindow<Plane>::createObjectPreset (parent,
		dbManager, flarmNetRecord.toPlane (), &paneData, NULL);
}

/**
 * Tries to identify the plane for an automatically created flight, asking the
 * user to choose or create a plane if necessary
 *
 * This method returns a plane ID if a plane has been identified and selected or
 * created by the user. The plane ID may be the same as the flight already has.
 * If the user cancels, chooses not to use the identified plane, or
 * identification fails, an invalid ID is returned.
 *
 * If identification fails, a message is shown to the user, unless
 * messageOnFailure is false.
 *
 * The caller is responsible for setting the plane of the flight and updating
 * the flight in the database (if applicable) if the plane changed.
 */
dbId PlaneIdentification::interactiveIdentifyPlane (const Flight &flight)
{
	Cache &cache=dbManager.getCache ();

	try
	{
		// We can only do this for automatically created flights
		if (flight.getFlarmId ().isEmpty ())
		{
			notCreatedAutomaticallyMessage ();
			return invalidId;
		}

		// Look up the plane, and see what we've got
		PlaneLookup::Result result=PlaneLookup (cache).lookupPlane (flight.getFlarmId ());
		if (result.plane.isValid ())
		{
			// We got a plane. Offer the user to use it. Plane lookup guarantees
			// that the Flarm ID of a returned plane matches. Therefore, we
			// don't have to update its Flarm ID.
			if (queryUsePlane (result.plane.getValue ()))
				return result.plane->getId ();
			else
				return invalidId;
		}
		else if (result.flarmNetRecord.isValid ())
		{
			// Offer the user to create a plane with the FlarmNet data
			// There is no plane with that Flarm ID, or plane lookup would have
			// found it. Therefore, we don't have to check for a Flarm ID
			// conflict.
			if (queryCreatePlane (result.flarmNetRecord.getValue ()))
				return interactiveCreatePlane (result.flarmNetRecord.getValue ());
			else
				return invalidId;
		}
		else
		{
			// Identification failed
			if (messageOnFailure)
				identificationFailureMessage ();

			return invalidId;
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		return invalidId;
	}

	return invalidId;
}
