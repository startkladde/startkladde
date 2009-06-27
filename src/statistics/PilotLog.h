#ifndef _PilotLog_h
#define _PilotLog_h

#include <QString>

// XXX
#include <q3ptrlist.h>
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator
#include <QDateTime>

#include "src/FlightList.h"
#include "src/db/Database.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/time/Time.h"

class PilotLogEntry
{
	public:
		enum flight_instructor_mode { fim_no, fim_strict, fim_loose };

		PilotLogEntry ();

		QDate tag;
		QString muster;
		QString registration;
		QString flugzeugfuehrer;
		QString begleiter;
		QString startart;
		QString ort_start;
		QString ort_landung;
		Time zeit_start;
		Time zeit_landung;
		Time flugdauer;
		QString bemerkung;

		bool invalid;

		QString tag_string () const;
		QString zeit_start_string (bool no_letters=false) const;
		QString zeit_landung_string (bool no_letters=false) const;
		QString flugdauer_string () const;
};

void makePilotLogsDay (QPtrList<PilotLogEntry> &fb, Database *db, QDate date);
void makePilotLogPerson (QPtrList<PilotLogEntry> &fb, Database *db, QDate date, Person *person, QPtrList<Flight> &flights, PilotLogEntry::flight_instructor_mode fim=PilotLogEntry::fim_no);

#endif

