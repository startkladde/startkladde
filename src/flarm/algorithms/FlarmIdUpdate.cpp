#include "src/flarm/algorithms/FlarmIdUpdate.h"

#include <QWidget>

#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/db/DbManager.h"
#include "src/text.h"
#include "src/gui/dialogs.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/db/DbManager.h"
#include "src/flarm/algorithms/FlarmIdCheck.h"

FlarmIdUpdate::FlarmIdUpdate (DbManager &dbManager, bool manualOperation, QWidget *parent):
	dbManager (dbManager), manualOperation (manualOperation), parent (parent)
{
}

FlarmIdUpdate::~FlarmIdUpdate ()
{
}

void FlarmIdUpdate::notCreatedAutomaticallyMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("FlarmIdUpdate", "Update Flarm ID"),
		qApp->translate ("FlarmIdUpdate", "The Flarm ID cannot be "
			"update because this flight was not created automatically."));
}

void FlarmIdUpdate::noPlaneMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("FlarmIdUpdate", "Update Flarm ID"),
		qApp->translate ("FlarmIdUpdate", "The Flarm ID cannot be "
			"update because this flight does not have a plane."));
}

void FlarmIdUpdate::currentMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("FlarmIdUpdate", "Update Flarm ID"),
		qApp->translate ("FlarmIdUpdate", "The Flarm ID of the plane of this "
			"flight is already current."));
}

QMessageBox::StandardButton FlarmIdUpdate::queryUpdateFlarmId (const Plane &plane, const Flight &flight)
{
	QString title=qApp->translate ("FlarmIdUpdate", "Update Flarm ID?");
	QString text =qApp->translate ("FlarmIdUpdate", "The Flarm ID of the plane "
		"(%1) is different from the one of this flight (%2). Probably, the "
		"plane is wrong a new Flarm has been installed in the plane. Do you "
		"want to update the plane's Flarm ID in the database?")
		.arg (plane.flarmId)
		.arg (flight.getFlarmId ());

	if (manualOperation)
		return yesNoQuestionStandardButton (parent, title, text);
	else
		return yesNoCancelQuestion (parent, title, text);
}

bool FlarmIdUpdate::checkAndUpdate (Plane &plane, const Flight &flight)
{
	// Check for a Flarm ID conflict first

	// Store the old Flarm ID, we'll need it later
	QString oldFlarmId=plane.flarmId;

	// Check for conflicts and apply the new Flarm ID
	FlarmIdCheck flarmIdCheck (dbManager, parent);
	flarmIdCheck.interactiveCheck (flight.getFlarmId (), plane.getId (), plane.flarmId);
	bool checkResult=flarmIdCheck.interactiveApply (&plane.flarmId);

	// If the user canceled, return false
	if (!checkResult)
		return false;

	// Only update the plane if the Flarm ID actually changed
	if (plane.flarmId!=oldFlarmId)
		dbManager.updateObject (plane, parent);

	return true;
}

bool FlarmIdUpdate::interactiveUpdateFlarmId (const Flight &flight)
{
	Cache &cache=dbManager.getCache ();

	try
	{
		// We can only do this for automatically created flights
		if (flight.getFlarmId ().isEmpty ())
		{
			notCreatedAutomaticallyMessage ();
			return true;
		}

		// We can only do this if the flight has a plane
		if (!idValid (flight.getPlaneId ()))
		{
			noPlaneMessage ();
			return true;
		}

		// Retrieve the plane from the database
		Plane plane=cache.getObject<Plane> (flight.getPlaneId ());

		// If the plane's Flarm ID matches the flight's (i. e., it is already
		// current), do nothing (potentially display a message)
		if (plane.flarmId==flight.getFlarmId ())
		{
			if (manualOperation)
				currentMessage ();

			return true;
		}

		// OK, so the flight has a Flarm ID (i. e. it was created automatically)
		// which does not match the plane's Flarm ID. Maybe we want to update
		// the plane in the database.

		// FIXME test both cases
		if (isBlank (plane.flarmId))
		{
			// The plane has no Flarm ID so far. This is the common case of
			// adding a Flarm ID to a plane, and there's little danger of doing
			// it wrong, so we do it silently.
			plane.flarmId=flight.getFlarmId ();
			// FIXME only if there are no conflicts
			dbManager.updateObject (plane, parent);
			return true;
		}
		else
		{
			// The plane has a different Flarm ID. This probably means that the
			// Flarm of the plane was exchanged or swapped with another Flarm.
			// That, or the user has entered a wrong plane. We'll have to ask
			// the user how to proceed.

			// Do you want to update the plane's Flarm ID?
			QMessageBox::StandardButton updateQuestionResult=queryUpdateFlarmId (plane, flight);


			if (updateQuestionResult==QMessageBox::Yes)
			{
				// Yes, update the plane
				return checkAndUpdate (plane, flight);
			}
			else if (updateQuestionResult==QMessageBox::No)
			{
				// No, don't update the plane
				return true;
			}
			else
			{
				// Cancel
				return false;
			}
		}
	}
	catch (Cache::NotFoundException &)
	{
		// Something went wrong - allow continuing
		return true;
	}
	catch (OperationCanceledException &)
	{
		// TODO the cache may now be inconsistent
		// User canceled
		return false;
	}

	// Cannot happen
	return true;
}
