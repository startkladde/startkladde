/*
 * FlightModel.cpp
 *
 *  Created on: Aug 30, 2009
 *      Author: mherrman
 */

#include "FlightModel.h"

#include <cassert>

#include "src/model/Flight.h"
#include "src/db/DataStorage.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/color.h"
#include "src/gui/widgets/SkTableView.h"

#include "src/itemDataRoles.h"

/*
 * Default column widths for the flight table:
tbl_idx_registration,         95, "D-WWWW (WW)"
tbl_idx_flugzeug_typ,         75, "DR-400/180"
tbl_idx_flug_typ,             50, "Normal"
tbl_idx_pilot,               150, "XXXXXXXX, YYYYYY"
tbl_idx_begleiter,           150, "XXXXXXXX, YYYYYY"
tbl_idx_startart,             70, "SOF"
tbl_idx_startzeit,            60, "  Starten  "
tbl_idx_landezeit,            60, "  Landen  "
tbl_idx_flugdauer,            50, "00:00"
tbl_idx_landungen,            50, "00"
tbl_idx_startort,            100, "Rheinstetten"
tbl_idx_zielort,             100, "Rheinstetten"
tbl_idx_bemerkungen,         200, "Seilrissübung"
tbl_idx_abrechnungshinweis,   50, "Landegebühr bezahlt"
tbl_idx_datum,                50, "0000-00-00"
tbl_idx_id_display,           50, "12345"
 *
 */

FlightModel::FlightModel (DataStorage &dataStorage):
	dataStorage (dataStorage)
{
}

FlightModel::~FlightModel ()
{
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

	std::cout << "column is " << column << std::endl;
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
		case 6: return "launchTime";
		case 7: return "landingTime";
		case 8: return "flightDuration";
		case 9: return "numLandings";
		case 10: return "departureAirfield";
		case 11: return "destinationAirfield";
		case 12: return "comments";
		case 13: return "accountingNote";
		case 14: return "date";
		case 15: return "id";
	}

	assert (!"Unhandled column");
	return QString ();
}

QVariant FlightModel::data (const Flight &flight, int column, int role) const
{
	// TODO: the error check for the color is done for each cell and each time
	// the cursor is moved - maybe someone (who?) should cache the data
	// TODO isButtonRole and buttonTextRole should be in xxxData ()

	if (role==Qt::DisplayRole)
	{
		switch (column)
		{
			case 0: return registrationData (flight, role);
			case 1: return planeTypeData (flight, role);
			case 2: return flightTypeText (flight.flightType, lsTable);
			case 3: return pilotData (flight, role);
			case 4: return copilotData (flight, role);
			case 5: return launchMethodData (flight, role);
			case 6: return launchTimeData (flight, role);
			case 7: return landingTimeData (flight, role);
			case 8: return durationData (flight, role);
			case 9: return flight.numLandings;
			case 10: return flight.departureAirfield;
			case 11: return flight.destinationAirfield;
			case 12: return flight.comments;
			case 13: return flight.isTowflight()?QString ("(Siehe geschleppter Flug)"):flight.accountingNote;
			case 14: return flight.effdatum ();
			case 15: return (flight.isTowflight ()?QString ("(%1)"):QString ("%1")).arg (flight.getId ());
		}

		assert (false);
		return QVariant ();
	}
	else if (role==Qt::BackgroundRole)
	{
		Plane *plane=dataStorage.getNewObject<Plane> (flight.plane);
		LaunchMethod *launchMethod=dataStorage.getNewObject<LaunchMethod> (flight.launchMethod);

		bool error=flight.fehlerhaft (plane, NULL, launchMethod);

		delete plane;
		delete launchMethod;

		QColor flightColor=flug_farbe (flight.mode, error, flight.isTowflight (), flight.started, flight.landed);
		return QBrush (flightColor);
	}
	else if (role==isButtonRole)
	{
		if      (column==launchButtonColumn ()) { return flight.canStart (); }
		else if (column==  landButtonColumn ()) { return flight.canLand (); }
		else return false;
	}
	else if (role==buttonTextRole)
	{
		if (column==launchButtonColumn ())
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
		Plane plane=dataStorage.getObject<Plane> (flight.plane);
		return plane.fullRegistration ();
	}
	catch (DataStorage::NotFoundException)
	{
		return "???";
	}
}

QVariant FlightModel::planeTypeData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Plane plane=dataStorage.getObject<Plane> (flight.plane);
		return plane.type;
	}
	catch (DataStorage::NotFoundException)
	{
		return "???";
	}
}

QVariant FlightModel::pilotData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Person pilot=dataStorage.getObject<Person> (flight.pilot);
		return pilot.formalNameWithClub ();
	}
	catch (DataStorage::NotFoundException)
	{
		return flight.incompletePilotName ();
	}
}

QVariant FlightModel::copilotData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		if (flightTypeCopilotRecorded (flight.flightType))
		{
			Person copilot=dataStorage.getObject<Person> (flight.copilot);
			return copilot.formalNameWithClub ();
		}
		else if (flightTypeIsGuest (flight.flightType))
		{
			return "(Gast)";
		}
		else
		{
			return "-";
		}
	}
	catch (DataStorage::NotFoundException)
	{
		return flight.incompleteCopilotName ();
	}
}

QVariant FlightModel::launchMethodData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		if (!flight.startsHere ()) return "-";

		LaunchMethod launchMethod=dataStorage.getObject<LaunchMethod> (flight.launchMethod);

		return launchMethod.shortName;

		// Alternative: if (launchMethod.is_airtow () && !launchMethod.towplaneKnown) return towplane.registraion or "???"
	}
	catch (DataStorage::NotFoundException)
	{
		return "?";
	}
}

QVariant FlightModel::launchTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveStartTime ())
		return "-";
	else if  (!flight.hasStartTime ())
		return "";
	else
		return flight.launchTime.table_string ();
}

QVariant FlightModel::landingTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveLandingTime())
		return "-";
	else if (!flight.hasLandingTime ())
		return "";
	else
		return flight.landingTime.table_string ();
}

QVariant FlightModel::durationData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.hasDuration ())
		return "-";
	else
		return flight.flightDuration ().table_string (tz_none);
}
