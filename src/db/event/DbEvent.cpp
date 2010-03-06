/*
 * TODO:
 *   - add template generators a la Db::DbEvent::added<T> (id) and
 *     Db::DbEvent::added (flight)
 *   - pass a copy of the object, via QSharedPointer (nb Thread safety,
 *     probably make a copy on get)
 *   - rename changed to updated
 */
#include "DbEvent.h"

#include <iostream>
#include <cassert>

class Plane;
class Flight;
class Person;
class LaunchMethod;

namespace Db { namespace Event
{
	/**
	 * This stinks, but sending signals across threads requires queued events,
	 * queued events require qRegisterMetaType and qRegisterMetaType requires
	 * a default constructor.
	 *
	 * In order not to have to introduce "dummy" values for the enums, we
	 * construct an arbitrary DbEvent here.
	 *
	 * But, as it turns out, that default contructor is never called. So we can
	 * disallow use of this constructor by raising an assertion.
	 */
	DbEvent::DbEvent ():
		type (typeChange), table (tablePeople), id (invalidId)
	{
		assert (!"DbEvent default constructor called");
	}

	DbEvent::DbEvent (Type type, Table table, dbId id):
		type (type), table (table), id (id)
	{
	}

	// We may need this when we want to use Event in Queued signals
	//DbEvent::Event ():
	//	type (DbEvent::typeNone), table (DbEvent::tableNone), id (invalidId)
	//{
	//}


	QString DbEvent::toString ()
	{
		return QString ("db_event (type: %1, table: %2, id: %3)")
			.arg (typeString (type), tableString (table)).arg (id);
	}

	QString DbEvent::typeString (DbEvent::Type type)
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

	QString DbEvent::tableString (DbEvent::Table table)
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
	template<> DbEvent::Table DbEvent::getTable<Flight>       () { return tableFlights      ; }
	template<> DbEvent::Table DbEvent::getTable<Plane>        () { return tablePlanes       ; }
	template<> DbEvent::Table DbEvent::getTable<Person>       () { return tablePeople       ; }
	template<> DbEvent::Table DbEvent::getTable<LaunchMethod> () { return tableLaunchMethods; }
} }
