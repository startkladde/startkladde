#include "Flight.h"

#include <cassert>

#include <QApplication>
#include <QList>

#include "src/util/qString.h"
#include "src/notr.h"

QList<Flight::Type> Flight::listTypes (bool includeInvalid)
{
	if (includeInvalid)
		return listTypes (false) << typeNone;
	else
		return QList<Type> ()
			<< typeNormal << typeTraining2 << typeTraining1
			<< typeGuestPrivate << typeGuestExternal;
}

QString Flight::typeText (Type type, bool withShortcut)
{
	if (withShortcut)
	{
		switch (type)
		{
			case typeNone:          return notr ("-");
			case typeNormal:        return qApp->translate ("Flight::Type", "R - Regular flight"); //tr N - Normalflug
			case typeTraining2:     return qApp->translate ("Flight::Type", "2 - Two-seated training"); //tr 2 - Schulungsflug (doppelsitzig)
			case typeTraining1:     return qApp->translate ("Flight::Type", "1 - Solo training"); //tr 1 - Schulungsflug (einsitzig)
			case typeTow:           return qApp->translate ("Flight::Type", "T - Towflight"); //tr S - Schleppflug
			case typeGuestPrivate:  return qApp->translate ("Flight::Type", "P - Passenger flight"); //tr G - Gastflug (privat)
			case typeGuestExternal: return qApp->translate ("Flight::Type", "E - Passenger flight (extern)"); //tr E - Gastflug (extern)
			// No default
		}
	}
	else
	{
		switch (type)
		{
			case typeNone:          return notr ("-");
			case typeNormal:        return qApp->translate ("Flight::Type", "Regular flight"); //tr Normalflug
			case typeTraining2:     return qApp->translate ("Flight::Type", "Two-seated training"); //tr Schulungsflug (doppelsitzig)
			case typeTraining1:     return qApp->translate ("Flight::Type", "Solo training"); //tr Schulungsflug (einsitzig)
			case typeTow:           return qApp->translate ("Flight::Type", "Towflight"); //tr Schleppflug
			case typeGuestPrivate:  return qApp->translate ("Flight::Type", "Passenger flight"); //tr Gastflug (privat)
			case typeGuestExternal: return qApp->translate ("Flight::Type", "Passenger flight (external)"); //tr Gastflug (extern)
			// No default
		}
	}

	assert (!notr ("Unhandled type"));
	return notr ("?");
}

QString Flight::shortTypeText (Type type)
{
	switch (type)
	{
		case typeNone:          return notr ("-");
		case typeNormal:        return qApp->translate ("Flight::Type", "Regular"); //tr Normal
		case typeTraining2:     return qApp->translate ("Flight::Type", "Training (2)"); //tr Schul (2)
		case typeTraining1:     return qApp->translate ("Flight::Type", "Training (1)"); //tr Schul (1)
		case typeTow:           return qApp->translate ("Flight::Type", "Tow"); //tr Schlepp
		case typeGuestPrivate:  return qApp->translate ("Flight::Type", "Passenger"); //tr Gast (P)
		case typeGuestExternal: return qApp->translate ("Flight::Type", "Passenger (E)"); //tr Gast (E)
		// No default
	}

	assert (!notr ("Unhandled type"));
	return notr ("?");
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
	// TODO TR: capitalization?
	switch (type)
	{
		case typeTraining2:
		case typeTraining1:
			return qApp->translate ("Flight::Type", "student"); //tr Flugschüler //tr TODO
		case typeNone:
		case typeNormal:
		case typeGuestPrivate:
		case typeGuestExternal:
			return qApp->translate ("Flight::Type", "pilot"); //tr Pilot //tr TODO
		case typeTow:
			return qApp->translate ("Flight::Type", "towpilot"); //tr Schlepppilot //tr TODO
	}

	assert (false);
	return qApp->translate ("Flight::Type", "pilot"); //tr Pilot
}

QString Flight::typeCopilotDescription (Flight::Type type)
{
	switch (type)
	{
		case typeTraining2:
			return qApp->translate ("Flight::Type", "flight instructor"); //tr Fluglehrer //tr TODO
		case typeGuestPrivate:
		case typeGuestExternal:
			return qApp->translate ("Flight::Type", "passenger"); //tr Gast //tr TODO
		case typeNone:
		case typeNormal:
		case typeTow:
			return qApp->translate ("Flight::Type", "copilot"); //tr Begleiter //tr TODO
		case typeTraining1:
			return notr ("-");
	}

	assert (false);
	return qApp->translate ("Flight::Type", "copilot"); //tr Begleiter
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

bool Flight::typeIsTraining (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return false;
		case typeNormal: return false;
		case typeTraining2: return true;
		case typeTraining1: return true;
		case typeTow: return false;
		case typeGuestPrivate: return false;
		case typeGuestExternal: return false;
	}

	assert (false);
	return false;
}
