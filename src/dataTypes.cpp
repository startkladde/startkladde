#include "dataTypes.h"

#include <cassert>

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
		case lsCsv:
			switch (flightType)
			{
				case ftNone:          return "---";
				case ftNormal:        return "Normalflug";
				case ftTraining2:     return "Schulung (2)";
				case ftTraining1:     return "Schulung (1)";
				case ftTow:           return "Schlepp";
				case ftGuestPrivate:  return "Gastflug (P)";
				case ftGuestExternal: return "Gastflug (E)";
				default:              return "???";
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

bool begleiter_erlaubt (FlightType flugtyp)
	/*
	 * Finds out if a copilot is allowed _to be recorded in the database_ for a
	 * given flight type.
	 * Parameters:
	 *   - flugtyp: the flight type.
	 * Return value:
	 *   - if a copilot is allowed.
	 */
{
	if (flugtyp==ftTraining1) return false;
	if (flugtyp==ftGuestPrivate) return false;		// Datenbanktechnisch gesehen jedenfalls
	if (flugtyp==ftGuestExternal) return false;		// Datenbanktechnisch gesehen jedenfalls
	return true;
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
		case lsShort: case lsTable: case lsPrintout: case lsPilotLog: case lsLong: case lsCsv: case lsWithShortcut:
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


db_event_table TableFromEntityType (EntityType t)
	/*
	 * Finds out which Table to use in the db_change mechanism for a given
	 * Entity type (see EntityEditWindow).
	 * This is a workaround for the fact that there is some code duplication
	 * between these two variable types.
	 * Parameters:
	 *   - t: the Entity type.
	 * Return value:
	 *   - the Table to use.
	 */
{
	switch (t)
	{
		case st_none: return db_kein; break;
		case st_plane: return db_flugzeug; break;
		case st_person: return db_person; break;
		default: return db_kein; break;
	}
}


