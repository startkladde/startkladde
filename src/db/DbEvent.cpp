#include "DbEvent.h"

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

void DbEvent::dump ()
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
		VALUE (db_kein,     "db_kein    ")
		VALUE (db_person,   "db_person  ")
		VALUE (db_flug,     "db_flug    ")
		VALUE (db_flugzeug, "db_flugzeug")
		VALUE (db_alle,     "db_alle    ")
		DEFAULT
	} printf ("    ");
	printf ("id: %llu\n", id);
}

