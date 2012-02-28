#include "FlightModel.h"

#include <cassert>
#include <iostream>

#include <QApplication>
#include <QBrush>

#include "src/itemDataRoles.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/db/cache/Cache.h"
#include "src/util/qString.h"
#include "src/notr.h"

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
		case 0: return qApp->translate ("FlightModel", "Reg.");
		case 1: return qApp->translate ("FlightModel", "Model");
		case 2: return qApp->translate ("FlightModel", "Type");
		case 3: return qApp->translate ("FlightModel", "Pilot/Student");
		case 4: return qApp->translate ("FlightModel", "Copilot/FI");
		case 5: return qApp->translate ("FlightModel", "Launch method");
		case 6: return qApp->translate ("FlightModel", "Departure");
		case 7: return qApp->translate ("FlightModel", "Landing");
		case 8: return qApp->translate ("FlightModel", "Duration");
		case 9: return qApp->translate ("FlightModel", "Ldgs.");
		case 10: return qApp->translate ("FlightModel", "Departure location");
		case 11: return qApp->translate ("FlightModel", "Landing location");
		case 12: return qApp->translate ("FlightModel", "Comments");
		case 13: return qApp->translate ("FlightModel", "Accounting notes");
		case 14: return qApp->translate ("FlightModel", "Date");
		case 15: return qApp->translate ("FlightModel", "ID");
	}

	// Apparantly, an unhandled column can happen when the last flight is deleted
	return QVariant ();
}


QString FlightModel::columnName (int columnIndex) const
{
	switch (columnIndex)
	{
		case 0: return notr ("registration");
		case 1: return notr ("aircraftType");
		case 2: return notr ("flightType");
		case 3: return notr ("pilot");
		case 4: return notr ("copilot");
		case 5: return notr ("launchMethod");
		case 6: return notr ("departureTime");
		case 7: return notr ("landingTime");
		case 8: return notr ("flightDuration");
		case 9: return notr ("numLandings");
		case 10: return notr ("departureLocation");
		case 11: return notr ("landingLocation");
		case 12: return notr ("comments");
		case 13: return notr ("accountingNote");
		case 14: return notr ("date");
		case 15: return notr ("id");
	}

	assert (!notr ("Unhandled column"));
	return QString ();
}

QString FlightModel::sampleText (int columnIndex) const
{
	switch (columnIndex)
	{
		case 0: return qApp->translate ("FlightModel", "N99999 (WW)");
		case 1: return qApp->translate ("FlightModel", "DR-400/180");
		case 2: return qApp->translate ("FlightModel", "Passenger (E)");
		case 3: return qApp->translate ("FlightModel", "Xxxxxxxx, Yyyyyy (Twidd");
		case 4: return qApp->translate ("FlightModel", "Xxxxxxxx, Yyyyyy (Twidd");
		case 5: return qApp->translate ("FlightModel", "Launch method"); // Header text is longer than content
		// Improvement: use QStyle::PM_ButtonMargin for buttons
		case 6: return qApp->translate ("FlightModel", "  Depart  ");
		case 7: return qApp->translate ("FlightModel", "  Land  ");
		case 8: return qApp->translate ("FlightModel", "00:00");
		case 9: return qApp->translate ("FlightModel", "Ldgs."); // Header text is longer than content
		case 10: return qApp->translate ("FlightModel", "Twiddlethorpe");
		case 11: return qApp->translate ("FlightModel", "Twiddlethorpe");
		case 12: return qApp->translate ("FlightModel", "Cable break training");
		case 13: return qApp->translate ("FlightModel", "Landing fee paid");
		case 14: return qApp->translate ("FlightModel", "12/34/5678");
		case 15: return qApp->translate ("FlightModel", "12345");
	}

	assert (!notr ("Unhandled column"));
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
			case 13: return flight.isTowflight()?qApp->translate ("FlightModel", "(See glider flight)"):flight.getAccountingNotes ();
			case 14: return flight.effdatum ();
			case 15: return (flight.isTowflight ()?qnotr ("(%1)"):qnotr ("%1")).arg (flight.getId ());
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
		// TODO TR cache
		if (column==departButtonColumn ())
			return qApp->translate ("FlightModel", "Depart");
		else if (column==landButtonColumn ())
		{
			if (flight.isTowflight () && !flight.landsHere ())
				return qApp->translate ("FlightModel", "End");
			else
				return qApp->translate ("FlightModel", "Land");
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
		return notr ("???");
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
		return notr ("???");
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
			return qApp->translate ("FlightModel", "(Passenger)");
		}
		else
		{
			return notr ("-");
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
		if (!flight.departsHere ()) return notr ("-");

		// For towflights without launch method, assume self launch
		if (idInvalid (flight.getLaunchMethodId ()) && flight.isTowflight ()) return qApp->translate ("FlightModel", "SL", "Self launch");

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (flight.getLaunchMethodId ());

		return launchMethod.shortName;

		// Alternative: if (launchMethod.is_airtow () && !launchMethod.towplaneKnown) return towplane.registraion or "???"
	}
	catch (Cache::NotFoundException)
	{
		return notr ("?");
	}
}

QVariant FlightModel::departureTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveDepartureTime ())
		return notr ("-");
	else if  (!flight.hasDepartureTime ())
		return notr ("");
	else
		// TODO TR cache
		return flight.getDepartureTime ().toUTC ().time ().toString (qApp->translate ("FlightModel", "hh:mm"))+"Z";
}

QVariant FlightModel::landingTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveLandingTime())
		return notr ("-");
	else if (!flight.hasLandingTime ())
		return notr ("");
	else
		return flight.getLandingTime ().toUTC ().time ().toString (qApp->translate ("FlightModel", "hh:mm"))+notr ("Z");
}

QVariant FlightModel::durationData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.hasDuration ())
		return notr ("-");
	else
		return flight.flightDuration ().toString (qApp->translate ("FlightModel", "h:mm"));
}
