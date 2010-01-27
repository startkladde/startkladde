#include "dataTypes.h"

#include <cassert>

class Plane;
class Person;
class Flight;

// TODO: namespaces and separate files for flight type, flight mode etc.

QList<FlightType> listFlightTypes (bool include_invalid)
{
	QList<FlightType> result;

	if (include_invalid)
		result << ftNone;

	result << ftNormal << ftTraining2 << ftTraining1 << ftGuestPrivate << ftGuestExternal;

	return result;
}

QString flightTypeText (FlightType flightType, lengthSpecification lenspec)
{
	switch (lenspec)
	{
		case lsShort:
			switch (flightType)
			{
				case ftNone:          return "-";
				case ftNormal:        return "N";
				case ftTraining2:     return "S2";
				case ftTraining1:     return "S1";
				case ftTow:           return "F";
				case ftGuestPrivate:  return "G";
				case ftGuestExternal: return "E";
			}
		case lsPrintout:
			switch (flightType)
			{
				case ftNone:          return "-";
				case ftNormal:        return "N";
				case ftTraining2:     return "S2";
				case ftTraining1:     return "S1";
				case ftTow:           return "F";
				case ftGuestPrivate:  return "G";
				case ftGuestExternal: return "E";
			}
		case lsTable: case lsPilotLog:
			switch (flightType)
			{
				case ftNone:          return "-";
				case ftNormal:        return "Normal";
				case ftTraining2:     return "Schul (2)";
				case ftTraining1:     return "Schul (1)";
				case ftTow:           return "Schlepp";
				case ftGuestPrivate:  return "Gast (P)";
				case ftGuestExternal: return "Gast (E)";
			}
		case lsLong:
			switch (flightType)
			{
				case ftNone:          return "---";
				case ftNormal:        return "Normalflug";
				case ftTraining2:     return "Schulungsflug (doppelsitzig)";
				case ftTraining1:     return "Schulungsflug (einsitzig)";
				case ftTow:           return "Schleppflug";
				case ftGuestPrivate:  return "Gastflug (privat)";
				case ftGuestExternal: return "Gastflug (extern)";
			}
		case lsWithShortcut:
			switch (flightType)
			{
				case ftNone:          return "---";
				case ftNormal:        return "N - Normalflug";
				case ftTraining2:     return "2 - Schulungsflug (doppelsitzig)";
				case ftTraining1:     return "1 - Schulungsflug (einsitzig)";
				case ftTow:           return "S - Schleppflug";
				case ftGuestPrivate:  return "G - Gastflug (privat)";
				case ftGuestExternal: return "E - Gastflug (extern)";
				default:              return "???";
			}
	}

	// We must have returned by now - the compiler should catch unhandled
	// values.
	assert (false);
}

QString flightTypePilotDescription (FlightType flightType)
{
	switch (flightType)
	{
		case ftTraining2:
		case ftTraining1:
			return QString::fromUtf8 ("Flugschüler");
		case ftNone:
		case ftNormal:
		case ftGuestPrivate:
		case ftGuestExternal:
			return QString ("Pilot");
		case ftTow:
			return QString ("Schlepppilot");
	}

	// Error: all cases must be handled
	return QString ("Pilot");
}

QString flightTypeCopilotDescription (FlightType flightType)
{
	switch (flightType)
	{
		case ftTraining2:
			return QString::fromUtf8 ("Fluglehrer");
		case ftGuestPrivate:
		case ftGuestExternal:
			return QString ("Gast");
		case ftNone:
		case ftNormal:
		case ftTow:
			return QString ("Begleiter");
		case ftTraining1:
			return QString ("-");
	}

	// Error: all cases must be handled
	return QString ("Begleiter");
}


bool flightTypeCopilotRecorded (FlightType flightType)
	/*
	 * Finds out if a copilot is allowed _to be recorded in the database_ for a
	 * given flight type.
	 * Parameters:
	 *   - flightType: the flight type.
	 * Return value:
	 *   - if a copilot is allowed.
	 */
{
	switch (flightType)
	{
		case ftNone: return true;
		case ftNormal: return true;
		case ftTraining2: return true;
		case ftTraining1: return false;
		case ftTow: return true;
		case ftGuestPrivate: return false;
		case ftGuestExternal: return false;
	}

	assert (false);
	return false;
}

/**
 * Whether a flight type always has a copilot
 */
bool flightTypeAlwaysHasCopilot (FlightType flightType)
{
	switch (flightType)
	{
		case ftNone: return false;
		case ftNormal: return false;
		case ftTraining2: return true;
		case ftTraining1: return false;
		case ftTow: return false;
		case ftGuestPrivate: return true;
		case ftGuestExternal: return true;
	}

	assert (false);
	return false;
}

bool flightTypeIsGuest (FlightType flightType)
{
	switch (flightType)
	{
		case ftNone: return false;
		case ftNormal: return false;
		case ftTraining2: return false;
		case ftTraining1: return false;
		case ftTow: return false;
		case ftGuestPrivate: return true;
		case ftGuestExternal: return true;
	}

	assert (false);
	return false;
}


QList<FlightMode> listFlightModes (bool include_invalid)
{
	QList<FlightMode> result;

	if (include_invalid)
		result << fmNone;

	result << fmLocal << fmComing << fmLeaving;

	return result;
}

QList<FlightMode> listTowFlightModes (bool include_invalid)
{
	QList<FlightMode> result;

	if (include_invalid)
		result << fmNone;

	result << fmLocal << fmLeaving;

	return result;
}

QString flightModeText (FlightMode modus, lengthSpecification lenspec)
{
	// The text is the same for all length specifications. Use a switch
	// statement anyway to enable compiler warnings in case another length
	// specification is added.
	switch (lenspec)
	{
		case lsShort: case lsTable: case lsPrintout: case lsPilotLog: case lsLong: case lsWithShortcut:
			switch (modus)
			{
				case fmNone:    return "-";
				case fmLocal:   return "Lokal";
				case fmComing:  return "Kommt";
				case fmLeaving: return "Geht";
			}
	}

	// We must have returned by now - the compiler should catch unhandled
	// values.
	assert (false);
}



bool ist_schulung (FlightType t)
	/*
	 * Finds out if a given flight type is training.
	 * Parameters:
	 *   - t: the flight type.
	 * Return value:
	 *   - true if the type is training.
	 */
{
	return (
		t==ftTraining2 ||
		t==ftTraining1 ||
		false);
}

bool lands_here (FlightMode m)
	/*
	 * Finds out if a flight of a given mode lands here.
	 * Parameters:
	 *   - m: the mode.
	 * Return value:
	 *   - true if the flight lands here.
	 */
{
	return (
		m==fmLocal ||
		m==fmComing ||
		false);
}

bool starts_here (FlightMode m)
	/*
	 * Finds out if a flight of a given mode starts here.
	 * Parameters:
	 *   - m: the mode.
	 * Return value:
	 *   - true if the flight lands here.
	 */
{
	return (
		m==fmLocal ||
		m==fmLeaving ||
		false);
}



QString std2q (std::string s)
	/*
	 * Converts a std::QString to a QString.
	 * Parameters:
	 *   - s: the source.
	 * Return value:
	 *   the converted QString.
	 */
{
	return QString (s.c_str ());
}

std::string q2std (QString s)
	/*
	 * Converts a QString to a std::QString
	 * Parameters:
	 *   - s: the source.
	 * Return value:
	 *   the converted QString.
	 */
{
	if (s.isNull ()) return "";
	return std::string (s.toLatin1 ().constData ());
}

//// Specialize getDbObjectType templates
//// We probably don't want to implement the generic one, as probably, any
//// function that calls this, doesn't make much sense with classes for which
//// there is no db_object_type.
////template<class T> db_object_type getDbObjectType  () { return ot_none  ; }
template<> db_event_table getDbEventTable<Flight> () { return db_flug; }
template<> db_event_table getDbEventTable<Plane > () { return db_flugzeug; }
template<> db_event_table getDbEventTable<Person> () { return db_person; }
