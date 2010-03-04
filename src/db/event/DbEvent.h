#ifndef DBEVENT_H_
#define DBEVENT_H_

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
		class DbEvent
		{
			public:
				enum Table { tablePeople, tableFlights, tableLaunchMethods, tablePlanes };
				enum Type { typeAdd, typeDelete, typeChange };

				DbEvent (Type type, Table table, dbId);

				QString toString ();
				static QString typeString (Type type);
				static QString tableString (Table table);

				template<class T> static Table getTable ();

				Type type;
				Table table;
				dbId id;
		};
	}
}

#endif
