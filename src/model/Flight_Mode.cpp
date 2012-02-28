#include "Flight.h"

#include <QApplication>
#include <QList>

#include <cassert>

#include "src/notr.h"

QList<Flight::Mode> Flight::listModes ()
{
	return QList<Mode> () << modeLocal << modeComing << modeLeaving;
}

QList<Flight::Mode> Flight::listTowModes ()
{
	return QList<Mode> () << modeLocal << modeLeaving;
}

QString Flight::modeText (Flight::Mode mode)
{
	switch (mode)
	{
		case modeLocal:   return qApp->translate ("Flight::Mode", "Local"); //tr Lokal
		case modeComing:  return qApp->translate ("Flight::Mode", "Coming"); //tr Kommt
		case modeLeaving: return qApp->translate ("Flight::Mode", "Leaving"); //tr Geht
		// no default
	}

	assert (!notr ("Unhandled mode"));
	return notr ("?");
}

bool Flight::landsHere (Flight::Mode mode)
{
	return mode==modeLocal || mode==modeComing;
}

bool Flight::departsHere (Flight::Mode mode)
{
	return mode==modeLocal || mode==modeLeaving;
}
