#include "Flight.h"

#include <cassert>

QList<Flight::Mode> Flight::listModes (bool includeInvalid)
{
	if (includeInvalid)
		return listModes (false) << modeNone;
	else
		return QList<Mode> () << modeLocal << modeComing << modeLeaving;
}

QList<Flight::Mode> Flight::listTowModes (bool includeInvalid)
{
	if (includeInvalid)
		return listModes (false) << modeNone;
	else
		return QList<Mode> () << modeLocal << modeLeaving;
}

QString Flight::modeText (Flight::Mode mode, lengthSpecification lenspec)
{
	// The text is the same for all length specifications. Use a switch
	// statement anyway to enable compiler warnings in case another length
	// specification is added.
	switch (lenspec)
	{
		case lsShort: case lsTable: case lsPrintout: case lsPilotLog: case lsLong: case lsWithShortcut:
		{
			switch (mode)
			{
				case modeNone:    return "-";
				case modeLocal:   return "Lokal";
				case modeComing:  return "Kommt";
				case modeLeaving: return "Geht";
			}
		}
	}

	// We must have returned by now - the compiler should catch unhandled
	// values.
	assert (false);
	return "?";
}

bool Flight::landsHere (Flight::Mode mode)
{
	return mode==modeLocal || mode==modeComing;
}

bool Flight::departsHere (Flight::Mode mode)
{
	return mode==modeLocal || mode==modeLeaving;
}
