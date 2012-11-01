#include "src/flarm/FlarmIdCheck.h"

#include "src/db/DbManager.h"
#include "src/model/Plane.h"
#include "src/gui/windows/input/ChoiceDialog.h"
#include "src/concurrent/monitor/OperationCanceledException.h"

FlarmIdCheck::FlarmIdCheck (DbManager &dbManager, QWidget *parent):
	dbManager (dbManager), parent (parent),
	conflict (false)
{
}

FlarmIdCheck::~FlarmIdCheck ()
{
}

QString FlarmIdCheck::Reaction_getText (Reaction reaction, const Plane &conflictingPlane,
	const QString &newFlarmId, const QString &oldFlarmId)
{
	(void)newFlarmId;

	switch (reaction)
	{
		case clear:
			return tr ("Clear the Flarm ID of %1")
				.arg (conflictingPlane.registration);
		case swap:
			return tr ("Swap the Flarm ID with %1")
				.arg (conflictingPlane.registration);
		case ignore:
			return tr ("Ignore the conflict - automatic departures and "
				"landings may not work correctly");
		case keep:
			return tr ("Keep this plane's Flarm ID at its old value, %1")
				.arg (oldFlarmId);
		case cancel:
			return tr ("Cancel");

		// No default
	}

	return "";
}

FlarmIdCheck::Options FlarmIdCheck::getOptions ()
{
	// cancel is never returned. It is not offered along with the other choices
	// but represents the user canceling the choice dialog.

	Options result;

	if (newFlarmId!=oldFlarmId)
	{
		// The Flarm ID changed (new==other; new!=old; new!=""). The options
		// are:
		//   * clear other     (this=new, other="")
		//   * swap with other (this=new, other=old) (only if old!="")
		//   * keep            (this=old, other=other)
		//   * ignore          (this=new, other=other)
		result.reactions << clear;
		if (!oldFlarmId.isEmpty ())
			result.reactions << swap;
		result.reactions << keep;
		result.reactions << ignore;
		result.defaultReaction=clear;
	}
	else
	{
		// The Flarm ID did not change (new==old==other!=""). This limits the
		// useful options to:
		//   * clear other     (this=new, other="")
		//   * ignore          (this=new, other=other)
		result.reactions << clear;
		result.reactions << ignore;
		result.defaultReaction=ignore;
	}

	return result;
}

FlarmIdCheck::Reaction FlarmIdCheck::showChoiceDialog (const Options &options)
{
	// Choice dialog title and text
	QString title=tr ("Flarm ID conflict");
	QString text=tr (
		"<html><b>The entered Flarm ID, %1, is already used for the plane %2.</b></html>")
		.arg (newFlarmId)
		.arg (conflictingPlane.registrationWithType ());

	QStringList choiceOptions;
	int defaultChoiceOption;

	for (int i=0, n=options.reactions.size (); i<n; ++i)
	{
		choiceOptions << Reaction_getText (options.reactions[i], conflictingPlane, newFlarmId, oldFlarmId);

		if (options.reactions[i]==options.defaultReaction)
			defaultChoiceOption=i;
	}

	int result=ChoiceDialog::choose (title, text, choiceOptions, defaultChoiceOption, parent);

	if (result<0)
		return cancel;
	else
		return options.reactions[result];
}

/**
 *
 * @param newFlarmId
 * @param oldFlarmId
 * @return true if OK, false if canceled
 */
bool FlarmIdCheck::interactiveCheck (const QString &newFlarmId, const QString &oldFlarmId)
{
	this->newFlarmId=newFlarmId;
	this->oldFlarmId=oldFlarmId;
	conflict=false;

	Cache &cache=dbManager.getCache ();

	// Nothing to do if the new Flarm ID is empty
	if (newFlarmId.trimmed ().isEmpty ())
		return true;

	// Find the conflicting plane, i. e. the plane that alrady has the Flarm ID
	// we're trying to set.
	// FIXME: the old Flarm ID may also be a duplicate
	// FIXME: if the Flarm ID did not change, we may get the same plane here
	dbId conflictingPlaneId=cache.getPlaneIdByFlarmId (newFlarmId);

	// Nothing to do if there is no conflict
	if (!idValid (conflictingPlaneId))
		return true;

	// There is a conflict
	conflict=true;

	// Retrieve the conflicting plane from the database
	// TODO NotFoundException (cannot happen)
	conflictingPlane=cache.getObject<Plane> (conflictingPlaneId);

	// Determine the available options
	Options options=getOptions ();

	selectedReaction=showChoiceDialog (options);

	if (selectedReaction==cancel)
		return false;
	else
		return true;
}

bool FlarmIdCheck::interactiveApply (QString *flightFlarmId)
{
	// By default, use the new Flarm ID for the flight. We'll change that later
	// if required by the user's choice.
	if (flightFlarmId)
		(*flightFlarmId)=newFlarmId;

	// Nothing to do if there is not conflict
	if (!conflict)
		return true;

	bool setOtherFlightFlarmId=false;
	QString otherFlightFlarmId;
	bool canceled=false;

	// There was a conflict. Let's see what the user chose to do.
	switch (selectedReaction)
	{
		case clear:
			setOtherFlightFlarmId=true;
			otherFlightFlarmId="";
			break;
		case swap:
			setOtherFlightFlarmId=true;
			otherFlightFlarmId=oldFlarmId;
			break;
		case keep:
			(*flightFlarmId)=oldFlarmId;
			break;
		case ignore:
			break;
		case cancel:
			canceled=true;
			break;
		// No default
	}

	if (setOtherFlightFlarmId)
	{
		try
		{
			conflictingPlane.flarmId=otherFlightFlarmId;
			dbManager.updateObject (conflictingPlane, parent);
		}
		catch (OperationCanceledException &)
		{
			// TODO the cache may now be inconsistent
		}
	}

	return !canceled;
}
