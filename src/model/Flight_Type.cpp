#include "Flight.h"

#include <cassert>

QList<Flight::Type> Flight::listTypes (bool includeInvalid)
{
	if (includeInvalid)
		return listTypes (false) << typeNone;
	else
		return QList<Type> ()
			<< typeNormal << typeTraining2 << typeTraining1
			<< typeGuestPrivate << typeGuestExternal;
}

QString Flight::typeText (Flight::Type type, lengthSpecification lenspec)
{
	switch (lenspec)
	{
		case lsShort:
			switch (type)
			{
				case typeNone:          return "-";
				case typeNormal:        return "N";
				case typeTraining2:     return "S2";
				case typeTraining1:     return "S1";
				case typeTow:           return "F";
				case typeGuestPrivate:  return "G";
				case typeGuestExternal: return "E";
			}
		case lsPrintout:
			switch (type)
			{
				case typeNone:          return "-";
				case typeNormal:        return "N";
				case typeTraining2:     return "S2";
				case typeTraining1:     return "S1";
				case typeTow:           return "F";
				case typeGuestPrivate:  return "G";
				case typeGuestExternal: return "E";
			}
		case lsTable: case lsPilotLog:
			switch (type)
			{
				case typeNone:          return "-";
				case typeNormal:        return "Normal";
				case typeTraining2:     return "Schul (2)";
				case typeTraining1:     return "Schul (1)";
				case typeTow:           return "Schlepp";
				case typeGuestPrivate:  return "Gast (P)";
				case typeGuestExternal: return "Gast (E)";
			}
		case lsLong:
			switch (type)
			{
				case typeNone:          return "---";
				case typeNormal:        return "Normalflug";
				case typeTraining2:     return "Schulungsflug (doppelsitzig)";
				case typeTraining1:     return "Schulungsflug (einsitzig)";
				case typeTow:           return "Schleppflug";
				case typeGuestPrivate:  return "Gastflug (privat)";
				case typeGuestExternal: return "Gastflug (extern)";
			}
		case lsWithShortcut:
			switch (type)
			{
				case typeNone:          return "---";
				case typeNormal:        return "N - Normalflug";
				case typeTraining2:     return "2 - Schulungsflug (doppelsitzig)";
				case typeTraining1:     return "1 - Schulungsflug (einsitzig)";
				case typeTow:           return "S - Schleppflug";
				case typeGuestPrivate:  return "G - Gastflug (privat)";
				case typeGuestExternal: return "E - Gastflug (extern)";
				default:              return "???";
			}
	}

	// We must have returned by now - the compiler should catch unhandled
	// values.
	assert (false);
}

// TODO rename allowed
bool Flight::typeCopilotRecorded (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return true;
		case typeNormal: return true;
		case typeTraining2: return true;
		case typeTraining1: return false;
		case typeTow: return true;
		case typeGuestPrivate: return false;
		case typeGuestExternal: return false;
	}

	assert (false);
	return false;
}

bool Flight::typeAlwaysHasCopilot (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return false;
		case typeNormal: return false;
		case typeTraining2: return true;
		case typeTraining1: return false;
		case typeTow: return false;
		case typeGuestPrivate: return true;
		case typeGuestExternal: return true;
	}

	assert (false);
	return false;
}

QString Flight::typePilotDescription (Flight::Type type)
{
	switch (type)
	{
		case typeTraining2:
		case typeTraining1:
			return QString::fromUtf8 ("Flugschüler");
		case typeNone:
		case typeNormal:
		case typeGuestPrivate:
		case typeGuestExternal:
			return QString ("Pilot");
		case typeTow:
			return QString ("Schlepppilot");
	}

	assert (false);
	return QString ("Pilot");
}

QString Flight::typeCopilotDescription (Flight::Type type)
{
	switch (type)
	{
		case typeTraining2:
			return QString::fromUtf8 ("Fluglehrer");
		case typeGuestPrivate:
		case typeGuestExternal:
			return QString ("Gast");
		case typeNone:
		case typeNormal:
		case typeTow:
			return QString ("Begleiter");
		case typeTraining1:
			return QString ("-");
	}

	assert (false);
	return QString ("Begleiter");
}

bool Flight::typeIsGuest (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return false;
		case typeNormal: return false;
		case typeTraining2: return false;
		case typeTraining1: return false;
		case typeTow: return false;
		case typeGuestPrivate: return true;
		case typeGuestExternal: return true;
	}

	assert (false);
	return false;
}
