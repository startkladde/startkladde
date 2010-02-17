#include "DbEvent.h"

//#include "src/model/Plane.h"
//#include "src/model/Flight.h"
//#include "src/model/Person.h"
//#include "src/model/LaunchMethod.h"

class Plane;
class Flight;
class Person;
class LaunchMethod;

DbEvent::DbEvent (db_event_type tp, db_event_table tb, db_id i)
	/*
	 * Initializes a DbEvent with given data.
	 * Parameters:
	 *   the data to set
	 */
{
	type=tp;
	table=tb;
	id=i;
}

DbEvent::DbEvent ()
	/*
	 * Initializes a DbEvent with default (empty/none) data.
	 */
{
	type=det_none;
	table=db_kein;
	id=0;
}

void DbEvent::dump () const
	/*
	 * Displays a dump of the event on stdout. Used for debugging.
	 */
{
	printf ("db_event dump:    ");
	printf ("type: ");
	switch (type)
	{
		VALUE (det_none,    "det_none   ")
		VALUE (det_add,     "det_add    ")
		VALUE (det_delete,  "det_delete ")
		VALUE (det_change,  "det_change ")
		VALUE (det_refresh, "det_refresh")
		DEFAULT
	} printf ("    ");
	printf ("table: ");
	switch (table)
	{
		VALUE (db_kein,          "db_kein         ")
		VALUE (db_person,        "db_person       ")
		VALUE (db_flug,          "db_flug         ")
		VALUE (db_flugzeug,      "db_flugzeug     ")
		VALUE (db_launch_method, "db_launch_method")
		VALUE (db_alle,          "db_alle         ")
		DEFAULT
	} printf ("    ");
	printf ("id: %llu\n", id);
}

// FIXME Not used lo ja, see data_types.cpp

// Specialize
template<> db_event_table DbEvent::getDbEventTable<Flight>       () { return db_flug         ; }
template<> db_event_table DbEvent::getDbEventTable<Plane>        () { return db_flugzeug     ; }
template<> db_event_table DbEvent::getDbEventTable<Person>       () { return db_person       ; }
template<> db_event_table DbEvent::getDbEventTable<LaunchMethod> () { return db_launch_method; }

// Instantiate
template db_event_table DbEvent::getDbEventTable<Flight      > ();
template db_event_table DbEvent::getDbEventTable<Plane       > ();
template db_event_table DbEvent::getDbEventTable<Person      > ();
template db_event_table DbEvent::getDbEventTable<LaunchMethod> ();
