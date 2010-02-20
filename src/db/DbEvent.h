#ifndef _DbEvent_h
#define _DbEvent_h

#include <QString>

#include "src/db/dbId.h"

class DbEvent
{
	public:
		// TODO replace with templates
		enum Table { tableNone, tableAll, tablePeople, tableFlights, tableLaunchMethods, tablePlanes };
		enum Type { typeNone, typeAdd, typeDelete, typeChange, typeRefresh };

		DbEvent ();
		DbEvent (Type type, Table table, db_id);

		QString toString ();
		static QString typeString (Type type);
		static QString tableString (Table table);

		// Hack for until we have replaced Table with templates
		template<class T> static Table getTable () { return tableNone; } // TODO remove?
//		template<class T> static Table getTable ();

		Type type;
		Table table;
		db_id id;
};

#endif
