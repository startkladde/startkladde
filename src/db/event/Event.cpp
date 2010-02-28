/*
 * TODO:
 *   - add template generators a la Db::Event::added<T> (id) and
 *     Db::Event::added (flight)
 *   - pass a copy of the object, via QSharedPointer (nb Thread safety,
 *     probably make a copy on get)
 *   - rename changed to updated
 */
#include "Event.h"

#include <iostream>
#include <cassert>

class Plane;
class Flight;
class Person;
class LaunchMethod;

namespace Db { namespace Event
{
	Event::Event (Type type, Table table, dbId id):
		type (type), table (table), id (id)
	{
	}

	// We may need this when we want to use Event in Queued signals
	//Event::Event ():
	//	type (Event::typeNone), table (Event::tableNone), id (invalidId)
	//{
	//}


	QString Event::toString ()
	{
		return QString ("db_event (type: %1, table: %2, id: %3)")
			.arg (typeString (type), tableString (table)).arg (id);
	}

	QString Event::typeString (Event::Type type)
	{
		switch (type)
		{
			case typeAdd    : return "add";
			case typeDelete : return "delete";
			case typeChange : return "change";
			// no default
		}

		assert (!"Unhandled type");
		return "?";
	}

	QString Event::tableString (Event::Table table)
	{
		switch (table)
		{
			case tablePeople: return "people";
			case tableFlights: return "flights";
			case tableLaunchMethods: return "launch methods";
			case tablePlanes: return "planes";
		}

		assert (!"Unhandled table");
		return "?";
	}

	// Specialize
	template<> Event::Table Event::getTable<Flight>       () { return tableFlights      ; }
	template<> Event::Table Event::getTable<Plane>        () { return tablePlanes       ; }
	template<> Event::Table Event::getTable<Person>       () { return tablePeople       ; }
	template<> Event::Table Event::getTable<LaunchMethod> () { return tableLaunchMethods; }
} }
