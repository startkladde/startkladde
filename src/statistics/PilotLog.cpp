#include "PilotLog.h"

#include <QSet>

#include "src/model/LaunchMethod.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/db/DataStorage.h"


// ************************
// ** Entry construction **
// ************************

PilotLog::Entry::Entry ():
	valid (true)
{
}

PilotLog::Entry::~Entry ()
{
}

// **********************
// ** Entry formatting **
// **********************

QString PilotLog::Entry::dateText () const
{
	return date.toString (Qt::LocaleDate);
}

QString PilotLog::Entry::departureTimeText (bool noLetters) const
{
	return departureTime.to_string ("%H:%M", tz_utc, 0, noLetters);
}

QString PilotLog::Entry::landingTimeText (bool noLetters) const
{
	return landingTime.to_string ("%H:%M", tz_utc, 0, noLetters);
}

QString PilotLog::Entry::flightDurationText () const
{
	return flightDuration.to_string ("%H:%M", tz_timespan);
}


// ********************
// ** Entry creation **
// ********************

/**
 * Makes a log entry from a flight
 *
 * @param flight
 * @param dataStorage
 * @return
 */
PilotLog::Entry PilotLog::Entry::create (const Flight *flight, DataStorage &dataStorage)
{
	PilotLog::Entry entry;

	Plane        *plane       =dataStorage.getNewObject<Plane       > (flight->planeId        );
	Person       *pilot       =dataStorage.getNewObject<Person      > (flight->pilotId        );
	Person       *copilot     =dataStorage.getNewObject<Person      > (flight->copilotId      );
	LaunchMethod *launchMethod=dataStorage.getNewObject<LaunchMethod> (flight->launchMethodId );

	entry.date=flight->effdatum ();
	if (plane) entry.planeType=plane->type;
	if (plane) entry.planeRegistration=plane->registration;
	if (pilot) entry.pilot=pilot->formalName ();
	if (copilot) entry.copilot=copilot->formalName ();
	if (launchMethod) entry.launchMethod=launchMethod->logString;
	entry.departureLocation=flight->departureLocation;
	entry.landingLocation=flight->landingLocation;
	entry.departureTime=flight->departureTime; // TODO: check flight mode
	entry.landingTime=flight->landingTime; // TODO: check flight mode
	entry.flightDuration=flight->flightDuration (); // TODO: check flight mode
	entry.comments=flight->comments;

	entry.valid=flight->finished ();

	delete plane;
	delete pilot;
	delete copilot;
	delete launchMethod;

	return entry;
}


// ******************
// ** Construction **
// ******************

PilotLog::PilotLog (QObject *parent):
	QAbstractTableModel (parent)
{
}

PilotLog::~PilotLog ()
{
}


// **************
// ** Creation **
// **************

/**
 * Makes the log for one pilot from a list of flights. The list may contain
 * flights of other people.
 *
 * @param personId
 * @param flights
 * @param dataStorage
 * @param mode
 * @return
 */
PilotLog *PilotLog::createNew (dbId personId, const QList<Flight> &flights, DataStorage &dataStorage, FlightInstructorMode mode)
{
	QList<const Flight *> interestingFlights;

	// Make a list of flights for this person
	foreach (const Flight &flight, flights)
	{
		if (flight.finished ())
		{
			// The person can be the pilot, or (depending on the flight instructor
			// mode) the flight instructor, which is the copilot)
			if (flight.pilotId==personId ||
				(mode==flightInstructorLoose && flight.copilotId==personId) ||
				(mode==flightInstructorStrict && flight.type==Flight::typeTraining2 && flight.copilotId==personId))
			{
				interestingFlights.append (&flight);
			}
		}
	}

	qSort (interestingFlights);

	// Iterate over all interesting flights, generating logbook entries.
	PilotLog *result=new PilotLog;
	foreach (Flight const *flight, interestingFlights)
		result->entries.append (PilotLog::Entry::create (flight, dataStorage));

	return result;
}

/**
 * Makes the logs for all pilots that have flights in a given flight list.
 *
 * @param flights
 * @param dataStorage
 * @return
 */
PilotLog *PilotLog::createNew (const QList<Flight> &flights, DataStorage &dataStorage, FlightInstructorMode mode)
{
	QSet<dbId> personIdSet;

	// Determine all people wo have flights
	foreach (const Flight &flight, flights)
	{
		if (flight.finished ())
		{
			personIdSet.insert (flight.pilotId);
			personIdSet.insert (flight.copilotId);
		}
	}

	QList<dbId> personIds=personIdSet.toList ();
	personIds.removeAll (0);

	// Make a list of the people and sort it
	QList<Person> people;
	foreach (const dbId &id, personIds)
	{
		try
		{
			people.append (dataStorage.getObject<Person> (id));
		}
		catch (...)
		{
			// TODO log error
		}
	}
	qSort (people);

	PilotLog *result=new PilotLog;
	foreach (const Person &person, people)
	{
		PilotLog *personResult=createNew (person.getId (), flights, dataStorage, mode);
		result->entries+=personResult->entries;
		delete personResult;
	}

	return result;
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

int PilotLog::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return entries.size ();
}

int PilotLog::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 12;
}

QVariant PilotLog::data (const QModelIndex &index, int role) const
{
	const Entry &entry=entries[index.row ()];

	// TODO: when invalid, add parentheses around depTime, landTime, flightDur and add comment

	if (role==Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0: return entry.dateText ();
			case 1: return entry.planeType;
			case 2: return entry.planeRegistration;
			case 3: return entry.pilot;
			case 4: return entry.copilot;
			case 5: return entry.launchMethod;
			case 6: return entry.departureLocation;
			case 7: return entry.landingLocation;
			case 8: return entry.departureTimeText ();
			case 9: return entry.landingTimeText ();
			case 10: return entry.flightDurationText ();
			case 11: return entry.comments;
			default: assert (false); return QVariant ();
		}
	}
	else
		return QVariant ();
}

QVariant PilotLog::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role==Qt::DisplayRole)
	{
		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
				case 0: return "Datum"; break;
				case 1: return "Typ"; break;
				case 2: return "Kennzeichen"; break;
				case 3: return "Pilot"; break;
				case 4: return "Begleiter"; break;
				case 5: return "Startart"; break;
				case 6: return "Startort"; break;
				case 7: return "Zielort"; break;
				case 8: return "Startzeit"; break;
				case 9: return "Landezeit"; break;
				case 10: return "Flugdauer"; break;
				case 11: return "Bemerkungen"; break;
			}
		}
		else
		{
			return section+1;
		}
	}

	return QVariant ();
}
