#include "dbTypes.h"

class Person;
class Flight;
class Plane;

bool id_valid (db_id id)
{
	return !id_invalid (id);
}

bool id_invalid (db_id id)
{
	return (id==0);
}


//// Specialize getDbObjectType templates
//// We probably don't want to implement the generic one, as probably, any
//// function that calls this, doesn't make much sense with classes for which
//// there is no db_object_type.
////template<class T> db_object_type getDbObjectType  () { return ot_none  ; }
//template<> db_object_type getDbObjectType<Flight> () { return ot_flight; }
//template<> db_object_type getDbObjectType<Plane > () { return ot_plane ; }
//template<> db_object_type getDbObjectType<Person> () { return ot_person; }
