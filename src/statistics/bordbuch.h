#ifndef bordbuch_h
#define bordbuch_h

/*
 * bordbuch
 * Martin Herrmann
 * 2004-09-28
 */

#include <QString>

// XXX
#include <q3ptrlist.h>
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator
#include <QDateTime>

#include "src/flight_list.h"
#include "src/db/sk_db.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/time/sk_time_t.h"

class bordbuch_entry
{
	public:
		bordbuch_entry ();

		QString club;
		QString registration;
		QString flugzeug_typ;
		QDate datum;
		QString name;
		int insassen;
		QString ort_von;
		QString ort_nach;
		sk_time_t zeit_start;
		sk_time_t zeit_landung;
		int anzahl_landungen;
		sk_time_t betriebsdauer;
		QString bemerkungen;
		bool invalid;

		QString insassen_string () const;
		QString datum_string () const;
		QString zeit_start_string (bool no_letters=false) const;
		QString zeit_landung_string (bool no_letters=false) const;
		QString betriebsdauer_string () const;
		QString anzahl_landungen_string () const;
};

bool make_bordbuch_entry (bordbuch_entry *bb_entry, sk_db *db, QPtrList<Flight> &flights, Plane &fz, QDate date);
void make_bordbuch_plane (QPtrList<bordbuch_entry> &bb, sk_db *db, QDate date, Plane &fz, QPtrList<Flight> &flights);
void make_bordbuch_day (QPtrList<bordbuch_entry> &bb, sk_db *db, QDate date, QPtrList<Plane> planes, QPtrList<Flight> flights, QString *club=NULL);
void make_bordbuch_day (QPtrList<bordbuch_entry> &bb, sk_db *db, QDate date);

#endif

