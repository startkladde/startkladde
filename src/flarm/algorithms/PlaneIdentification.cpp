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

PlaneIdentification::PlaneIdentification ()
{
}

PlaneIdentification::~PlaneIdentification ()
{
}

bool PlaneIdentification::queryUsePlane (QWidget *parent, const Plane &plane)
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

bool PlaneIdentification::queryCreatePlane (QWidget *parent, const FlarmNetRecord &flarmNetRecord)
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

void PlaneIdentification::notCreatedAutomaticallyMessage (QWidget *parent)
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane cannot be "
			"identified because this flight was not created automatically."));
}

/**
 * Tries to identify the plane for an automatically created flight, asking the
 * user to choose or create a plane if necessary
 *
 * @param parent Qt parent widget for dialogs
 * @param dbManager the database to use for retrieving plane data
 * @param flarmId the Flarm ID of the flight
 * @return
 */
// FIXME: this returns the same value (invalidId) in three cases:
//   - user canceled
//   - identification failed
//   - user chose "no change"
dbId PlaneIdentification::interactiveIdentifyPlane (QWidget *parent, DbManager &dbManager, const QString &flarmId)
{
	Cache &cache=dbManager.getCache ();

	try
	{
		// We can only do this for automatically created flights
		if (flarmId.isEmpty ())
		{
			notCreatedAutomaticallyMessage (parent);
			return invalidId;
		}

		// Look up the plane
		PlaneLookup::Result result=PlaneLookup (cache).lookupPlane (flarmId);

		// Let's see what we've got...
		if (result.plane.isValid ())
		{
			// We got a plane. Offer the user to use it. Plane lookup guarantees
			// that the Flarm ID of a returned plane matches.
			if (queryUsePlane (parent, result.plane.getValue ()))
				return result.plane->getId ();
		}
		else if (result.flarmNetRecord.isValid ())
		{
			// Offer the user to create a plane with the FlarmNet data
			if (queryCreatePlane (parent, result.flarmNetRecord.getValue ()))
			{
				PlaneEditorPaneData paneData;
				paneData.flarmIdReadOnly=true;

				// There is no plane with that Flarm ID, or plane lookup would
				// have found it. Therefore, we don't have to check for a Flarm
				// ID conflict.
				return ObjectEditorWindow<Plane>::createObjectPreset (parent,
					dbManager, result.flarmNetRecord->toPlane (), &paneData, NULL);
			}
		}

		// Either we didn't find anything or the user declined the offer to use
		// or create the plane. In any case, continue without a plane. The user
		// will have to enter (and potentially create) the plane manually.
		return invalidId;
	}
	catch (Cache::NotFoundException &ex)
	{
		return invalidId;
	}

	return invalidId;
}


/**
 * Frontend method
*/
dbId PlaneIdentification::interactiveIdentifyPlane (QWidget *parent, DbManager &dbManager, dbId flightId)
{
	try
	{
		Flight flight=dbManager.getCache ().getObject<Flight> (flightId);
		return interactiveIdentifyPlane (parent, dbManager, flight.getFlarmId ());
	}
	catch (Cache::NotFoundException &ex)
	{
		return invalidId;
	}

	return invalidId;
}
