#ifndef DBEVENT_H_
#define DBEVENT_H_

#include <QString>
#include <QVariant>

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
				// ** Types
				enum Table { tablePeople, tableFlights, tableLaunchMethods, tablePlanes };
				enum Type { typeAdd, typeDelete, typeChange };

				// ** Construction
				DbEvent ();
				DbEvent (Type type, Table table, dbId id, const QVariant &value);

				template<class T> static DbEvent added (const T &object)
				{
					QVariant value;
					value.setValue (object);
					return DbEvent (typeAdd, getTable<T> (), object.getId (), value);
				}

				template<class T> static DbEvent changed (const T &object)
				{
					QVariant value;
					value.setValue (object);
					return DbEvent (typeChange, getTable<T> (), object.getId (), value);
				}

				template<class T> static DbEvent deleted (const T &object)
				{
					QVariant value;
					value.setValue (object);
					return DbEvent (typeDelete, getTable<T> (), object.getId (), value);
				}

				template<class T> static DbEvent deleted (dbId id)
				{
					return DbEvent (typeDelete, getTable<T> (), id, QVariant ());
				}


//				FIXME: template<T> static added, deleted, changed

				// ** Formatting
				QString toString ();
				static QString typeString (Type type);
				static QString tableString (Table table);

				// ** Property access
				Table    getTable () { return table; }
				Type     getType  () { return type ; }
				dbId     getId    () { return id   ; }
				QVariant getValue () { return type ; }

				template <class T> T getValue () { return value.value<T> (); }

				// ** Table methods
				template<class T> bool hasTable () { return table==getTable<T> (); }
				template<class T> static Table getTable ();

			private:
				Type type;
				Table table;
				dbId id;
				QVariant value;
		};
	}
}

#endif
