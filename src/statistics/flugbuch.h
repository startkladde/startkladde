#ifndef flugbuch_h
#define flugbuch_h

/*
 * flugbuch
 * Martin Herrmann
 * 2004-09-28
 */

// Qt3:
//#include <qptrlist.h>
// Qt4:
#include <q3ptrlist.h>
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator

#include <string>
#include <qdatetime.h>
#include "src/model/sk_flugzeug.h"
#include "src/model/sk_flug.h"
#include "src/model/sk_person.h"
#include "src/time/sk_time_t.h"
#include "src/db/sk_db.h"
#include "src/flight_list.h"

using namespace std;

class flugbuch_entry
{
	public:
		enum flight_instructor_mode { fim_no, fim_strict, fim_loose };

		flugbuch_entry ();

		QDate tag;
		string muster;
		string registration;
		string flugzeugfuehrer;
		string begleiter;
		string startart;
		string ort_start;
		string ort_landung;
		sk_time_t zeit_start;
		sk_time_t zeit_landung;
		sk_time_t flugdauer;
		string bemerkung;

		bool invalid;

		string tag_string () const;
		string zeit_start_string (bool no_letters=false) const;
		string zeit_landung_string (bool no_letters=false) const;
		string flugdauer_string () const;
};

void make_flugbuch_day (QPtrList<flugbuch_entry> &fb, sk_db *db, QDate date);
void make_flugbuch_person (QPtrList<flugbuch_entry> &fb, sk_db *db, QDate date, sk_person *person, QPtrList<sk_flug> &flights, flugbuch_entry::flight_instructor_mode fim=flugbuch_entry::fim_no);

#endif

