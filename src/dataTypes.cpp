#include "dataTypes.h"

#include <cassert>

class Plane;
class Person;
class Flight;


QString std2q (std::string s)
	/*
	 * Converts a std::QString to a QString.
	 * Parameters:
	 *   - s: the source.
	 * Return value:
	 *   the converted QString.
	 */
{
	return QString (s.c_str ());
}

std::string q2std (QString s)
	/*
	 * Converts a QString to a std::QString
	 * Parameters:
	 *   - s: the source.
	 * Return value:
	 *   the converted QString.
	 */
{
	if (s.isNull ()) return "";
	return std::string (s.toLatin1 ().constData ());
}

//// Specialize getDbObjectType templates
//// We probably don't want to implement the generic one, as probably, any
//// function that calls this, doesn't make much sense with classes for which
//// there is no db_object_type.
////template<class T> db_object_type getDbObjectType  () { return ot_none  ; }
template<> db_event_table getDbEventTable<Flight> () { return db_flug; }
template<> db_event_table getDbEventTable<Plane > () { return db_flugzeug; }
template<> db_event_table getDbEventTable<Person> () { return db_person; }
