#include "dataTypes.h"

#include <cassert>

class Plane;
class Person;
class Flight;
class LaunchMethod;


//// Specialize getDbObjectType templates
//// We probably don't want to implement the generic one, as probably, any
//// function that calls this, doesn't make much sense with classes for which
//// there is no db_object_type.
////template<class T> db_object_type getDbObjectType  () { return ot_none  ; }
// FIXME remove this, replacedd by DbEvent::...?
template<> db_event_table getDbEventTable<Flight> () { return db_flug; }
template<> db_event_table getDbEventTable<Plane > () { return db_flugzeug; }
template<> db_event_table getDbEventTable<Person> () { return db_person; }
template<> db_event_table getDbEventTable<LaunchMethod> () { return db_launch_method; }
