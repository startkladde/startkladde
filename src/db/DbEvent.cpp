#include "DbEvent.h"

#include <iostream>
#include <cassert>

class Plane;
class Flight;
class Person;
class LaunchMethod;

DbEvent::DbEvent (Type type, Table table, db_id id):
	type (type), table (table), id (id)
{
}

// TODO remove?
DbEvent::DbEvent ():
	type (DbEvent::typeNone), table (DbEvent::tableNone), id (invalid_id)
{
}


// FIXME remove
#define VALUE(x,z) case x: std::cout << (z); break;
#define DEFAULT default: std::cout << ("???"); break;

QString DbEvent::toString ()
{
	return QString ("db_event (type: %1, table: %2, id: %3)")
		.arg (typeString (type), tableString (table)).arg (id);
}

QString DbEvent::typeString (DbEvent::Type type)
{
	switch (type)
	{
		case typeNone   : return "none";
		case typeAdd    : return "add";
		case typeDelete : return "delete";
		case typeChange : return "change";
		case typeRefresh: return "refresh";
		// no default
	}

	assert (!"Unhandled type");
	return "?";
}

QString DbEvent::tableString (DbEvent::Table table)
{
	switch (table)
	{
		case tableNone: return "none";
		case tableAll: return "all";
		case tablePeople: return "people";
		case tableFlights: return "flights";
		case tableLaunchMethods: return "launch methods";
		case tablePlanes: return "planes";
	}

	assert (!"Unhandled table");
	return "?";
}



// Specialize
template<> DbEvent::Table DbEvent::getTable<Flight>       () { return tableFlights      ; }
template<> DbEvent::Table DbEvent::getTable<Plane>        () { return tablePlanes       ; }
template<> DbEvent::Table DbEvent::getTable<Person>       () { return tablePeople       ; }
template<> DbEvent::Table DbEvent::getTable<LaunchMethod> () { return tableLaunchMethods; }

//// Instantiate
//template db_event_table DbEvent::getDbEventTable<Flight      > ();
//template db_event_table DbEvent::getDbEventTable<Plane       > ();
//template db_event_table DbEvent::getDbEventTable<Person      > ();
//template db_event_table DbEvent::getDbEventTable<LaunchMethod> ();
