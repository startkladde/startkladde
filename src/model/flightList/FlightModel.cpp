#include "FlightModel.h"

#include <cassert>
#include <iostream>

#include <QBrush>

#include "src/itemDataRoles.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/db/cache/Cache.h"
#include "src/util/qString.h"

FlightModel::FlightModel (Cache &cache):
	cache (cache),
	colorEnabled (true)
{
}

FlightModel::~FlightModel ()
{
}

void FlightModel::setColorEnabled (bool colorEnabled)
{
	this->colorEnabled=colorEnabled;
}

int FlightModel::columnCount () const
{
	return 16;
}

QVariant FlightModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Kennz.";
		case 1: return "Typ";
		case 2: return "Flugtyp";
		case 3: return "Pilot/FS";
		case 4: return "Begleiter/FL";
		case 5: return "Startart";
		case 6: return "Start";
		case 7: return "Landung";
		case 8: return "Dauer";
		case 9: return "Ldg.";
		case 10: return "Startort";
		case 11: return "Zielort";
		case 12: return "Bemerkungen";
		case 13: return "Abrechnungshinweis";
		case 14: return "Datum";
		case 15: return "ID";
	}

	// Apparantly, an unhandled column can happen when the last flight is deleted
	return QVariant ();
}


QString FlightModel::columnName (int columnIndex) const
{
	switch (columnIndex)
	{
		case 0: return "registration";
		case 1: return "aircraftType";
		case 2: return "flightType";
		case 3: return "pilot";
		case 4: return "copilot";
		case 5: return "launchMethod";
		case 6: return "departureTime";
		case 7: return "landingTime";
		case 8: return "flightDuration";
		case 9: return "numLandings";
		case 10: return "departureLocation";
		case 11: return "landingLocation";
		case 12: return "comments";
		case 13: return "accountingNote";
		case 14: return "date";
		case 15: return "id";
	}

	assert (!"Unhandled column");
	return QString ();
}

QString FlightModel::sampleText (int columnIndex) const
{
	switch (columnIndex)
	{
		case 0: return "D-1234 (WW)";
		case 1: return "DR-400/180";
		case 2: return "Schul (2)";
		case 3: return "Xxxxxxxx, Yyyyyy (FSV Ding";
		case 4: return "Xxxxxxxx, Yyyyyy (FSV Ding";
		case 5: return "Startart"; // Header text is longer than content
		// Improvement: use QStyle::PM_ButtonMargin for buttons
		case 6: return "  Starten  ";
		case 7: return "  Landen  ";
		case 8: return "00:00";
		case 9: return "Ldg."; // Header text is longer than content
		case 10: return "Rheinstetten";
		case 11: return "Rheinstetten";
		case 12: return utf8 ("Seilrissübung");
		case 13: return utf8 ("Landegebühr bezahlt");
		case 14: return "12.34.5678";
		case 15: return "12345";
	}

	assert (!"Unhandled column");
	return QString ();
}

QVariant FlightModel::data (const Flight &flight, int column, int role) const
{
	// TODO more caching - this is called very often
	// TODO isButtonRole and buttonTextRole should be in xxxData ()

	if (role==Qt::DisplayRole)
	{
		switch (column)
		{
			case 0: return registrationData (flight, role);
			case 1: return planeTypeData (flight, role);
			case 2: return Flight::shortTypeText (flight.getType ());
			case 3: return pilotData (flight, role);
			case 4: return copilotData (flight, role);
			case 5: return launchMethodData (flight, role);
			case 6: return departureTimeData (flight, role);
			case 7: return landingTimeData (flight, role);
			case 8: return durationData (flight, role);
			case 9: return flight.getNumLandings ();
			case 10: return flight.getDepartureLocation ();
			case 11: return flight.getLandingLocation ();
			case 12: return flight.getComments ();
			case 13: return flight.isTowflight()?QString ("(Siehe geschleppter Flug)"):flight.getAccountingNotes ();
			case 14: return flight.effdatum ();
			case 15: return (flight.isTowflight ()?QString ("(%1)"):QString ("%1")).arg (flight.getId ());
		}

		assert (false);
		return QVariant ();
	}
	else if (role==Qt::BackgroundRole)
	{
		if (colorEnabled)
			return QBrush (flight.getColor (cache));
		else
			return QVariant ();
	}
	else if (role==isButtonRole)
	{
		// Only show buttons for prepared flights and today's flights
		if (!flight.isPrepared() && flight.getDepartureTime ().toLocalTime ().date ()!=QDate::currentDate ())
			return false;

		if      (column==departButtonColumn ()) { return flight.canDepart (); }
		else if (column==  landButtonColumn ()) { return flight.canLand (); }
		else return false;
	}
	else if (role==buttonTextRole)
	{
		if (column==departButtonColumn ())
			return "Starten";
		else if (column==landButtonColumn ())
		{
			if (flight.isTowflight () && !flight.landsHere ())
				return "Ende";
			else
				return "Landen";
		}
		return QVariant ();
	}

	return QVariant ();
}


// ******************
// ** Data methods **
// ******************

QVariant FlightModel::registrationData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Plane plane=cache.getObject<Plane> (flight.getPlaneId ());
		return plane.fullRegistration ();
	}
	catch (Cache::NotFoundException)
	{
		return "???";
	}
}

QVariant FlightModel::planeTypeData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Plane plane=cache.getObject<Plane> (flight.getPlaneId ());
		return plane.type;
	}
	catch (Cache::NotFoundException)
	{
		return "???";
	}
}

QVariant FlightModel::pilotData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Person pilot=cache.getObject<Person> (flight.getPilotId ());
		return pilot.formalNameWithClub ();
	}
	catch (Cache::NotFoundException)
	{
		return flight.incompletePilotName ();
	}
}

QVariant FlightModel::copilotData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		if (Flight::typeCopilotRecorded (flight.getType ()))
		{
			Person copilot=cache.getObject<Person> (flight.getCopilotId ());
			return copilot.formalNameWithClub ();
		}
		else if (Flight::typeIsGuest (flight.getType ()))
		{
			return "(Gast)";
		}
		else
		{
			return "-";
		}
	}
	catch (Cache::NotFoundException)
	{
		return flight.incompleteCopilotName ();
	}
}

QVariant FlightModel::launchMethodData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		if (!flight.departsHere ()) return "-";

		// For towflights without launch method, assume self launch
		if (idInvalid (flight.getLaunchMethodId ()) && flight.isTowflight ()) return "ES";

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (flight.getLaunchMethodId ());

		return launchMethod.shortName;

		// Alternative: if (launchMethod.is_airtow () && !launchMethod.towplaneKnown) return towplane.registraion or "???"
	}
	catch (Cache::NotFoundException)
	{
		return "?";
	}
}

QVariant FlightModel::departureTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveDepartureTime ())
		return "-";
	else if  (!flight.hasDepartureTime ())
		return "";
	else
		return flight.getDepartureTime ().toUTC ().time ().toString ("hh:mm")+"Z";
}

QVariant FlightModel::landingTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveLandingTime())
		return "-";
	else if (!flight.hasLandingTime ())
		return "";
	else
		return flight.getLandingTime ().toUTC ().time ().toString ("hh:mm")+"Z";
}

QVariant FlightModel::durationData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.hasDuration ())
		return "-";
	else
		return flight.flightDuration ().toString ("h:mm");
}
