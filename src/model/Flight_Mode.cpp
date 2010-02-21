#include "Flight.h"

#include <QList>

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

QString Flight::modeText (Flight::Mode mode)
{
	switch (mode)
	{
		case modeNone:    return "-";
		case modeLocal:   return "Lokal";
		case modeComing:  return "Kommt";
		case modeLeaving: return "Geht";
		// no default
	}

	assert (!"Unhandled mode");
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
