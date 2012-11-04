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

FlarmIdUpdate::FlarmIdUpdate (DbManager &dbManager, QWidget *parent):
	dbManager (dbManager), parent (parent),
	manualOperation (true), oldPlaneId (invalidId)
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

FlarmIdUpdate::UpdateAction FlarmIdUpdate::queryUpdateFlarmId (const Plane &plane, const Flight &flight)
{
	QString title=qApp->translate ("FlarmIdUpdate", "Update Flarm ID?");

	// TODO this could be more user-friendly, along the lines of:
	//   How do you want to proceed?
	//     ( ) Update the plane's Flarm ID. Choose this if a [new] Flarm was
	//         installed in the plane
	//     ( ) Don't update the plane's Flarm ID
	//     ( ) Go back and edit the plane. Choose this if the plane is wrong.
	// Also, we should have a "details" button which shows the Flarm IDs of the
	// plane and of the flight.
	QString text;
	if (isBlank (plane.flarmId))
	{
		text=qApp->translate ("FlarmIdUpdate",
				"The selected plane has no Flarm ID. "
				"This usually means that a Flarm has recently been installed "
				"in the plane. It could also mean that a wrong plane has been "
				"entered. "
				"Do you want to update the plane's Flarm ID in the database?");
	}
	else
	{
		text=qApp->translate ("FlarmIdUpdate",
			"The Flarm ID of the plane (%1) is different from the one of this "
			"flight (%2). "
			"This usually means that a new Flarm has been installed in the "
			"plane. It could also mean that a wrong plane has been entered. "
			"Do you want to update the plane's Flarm ID in the database?")
			.arg (plane.flarmId)
			.arg (flight.getFlarmId ());
	}

	QMessageBox::StandardButton result;
	if (manualOperation)
		result=yesNoQuestionStandardButton (parent, title, text);
	else
		result=yesNoCancelQuestion (parent, title, text);

	if (result==QMessageBox::Yes)
		return update;
	else if (result==QMessageBox::No)
		return dontUpdate;
	else
		return cancel;
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

/**
 * Checks if it is possible to update the plane's Flarm ID silently, i. e.
 * without asking the user
 *
 * This is relevant for one very common case: an incomplete plane (i. e. a plane
 * which is in the database, but without a Flarm ID) has departed, the flight
 * has been created automatically, and the user edits the flight and enters the
 * plane's registration. This case is so common, and the risk of doing it wrong
 * so low, that we may make an exception to the rule of not changing a flight
 * without asking the user.
 *
 * All of the following conditions must be fulfilled in order to update a plane
 * silently:
 *   - the plane has no Flarm ID yet
 *   - the Flarm ID of the flight is not taken yet (i. e. there is no plane with
 *     that Flarm ID yet)
 *   - the flight did not have a plane before
 *   - the operation was not performed manually
 */
bool FlarmIdUpdate::canUpdateSilently (const Plane &plane, const Flight &flight)
{
	// Note that we check in increasing order of computational effort

	if (manualOperation)
		return false;

	if (idValid (oldPlaneId))
		return false;

	if (!isBlank (plane.flarmId))
		return false;

	Cache &cache=dbManager.getCache ();
	bool flarmIdTaken=!cache.getPlaneIdsByFlarmId (flight.getFlarmId ()).isEmpty ();
	if (flarmIdTaken)
		return false;

	return true;
}

bool FlarmIdUpdate::interactiveUpdateFlarmId (const Flight &flight, bool manualOperation, dbId oldPlaneId)
{
	this->manualOperation=manualOperation;
	this->oldPlaneId=oldPlaneId;

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
		// which does not match the plane's Flarm ID. Either the plane is wrong,
		// or a new Flarm unit has been installed in the plane. Maybe we want to
		// update the plane in the database.

		// Under some very specific circumstances, we can perform the update
		// silently. Otherwise, we'll have to ask the user.
		UpdateAction updateAction;
		if (canUpdateSilently (plane, flight))
			updateAction=update;
		else
			updateAction=queryUpdateFlarmId (plane, flight);

		// Depending on the user's choice, do or don't update the plane, or
		// cancel the operation.
		switch (updateAction)
		{
			case update:
				// Yes, update the plane's Flarm ID
				// Note that when performing the update silently, we will still
				// check for conflicts, but we know there are none.
				return checkAndUpdate (plane, flight);
			case dontUpdate:
				// No, don't update the plane's Flarm ID
				return true;
			case cancel:
				// Cancel
				return false;
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
