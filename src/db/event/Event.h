#ifndef EVENT_H_
#define EVENT_H_

#include <QString>

#include "src/db/dbId.h"

namespace Db
{
	namespace Event
	{
		/**
		 * A description of a change in the database
		 *
		 * This is implemented with an Table Enum rather than as a template because it
		 * is sent as a parameter as a signal and signals can't be templates.
		 */
		class Event
		{
			public:
				enum Table { tableAll, tablePeople, tableFlights, tableLaunchMethods, tablePlanes };
				enum Type { typeAdd, typeDelete, typeChange, typeRefresh };

				Event (Type type, Table table, dbId);

				QString toString ();
				static QString typeString (Type type);
				static QString tableString (Table table);

				// Hack for until we have replaced Table with templates
				template<class T> static Table getTable ();

				Type type;
				Table table;
				dbId id;
		};
	}
}

#endif
