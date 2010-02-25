#ifndef DATABASE_H_
#define DATABASE_H_

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>
#include <QSqlError>

#include "src/db/dbId.h"
#include "src/db/DatabaseInfo.h"
#include "src/db/interface/DefaultInterface.h" // TODO remove (to worker)
#include "src/db/AbstractDatabase.h"

class Flight;

namespace Db
{
	/**
	 * Methods for manipulating objects in the database (ORM).
	 *
	 * Defines template methods for retrieving, creating, counting, deleting,
	 * creating and updating objects, which are instantiated for the relevant
	 * classes in the .cpp file. Also provides some frontends for object selection
	 * (like getFlightsDate).
	 *
	 * This class is NOT thread safe. It may only be used in the thread where it
	 * was created.
	 *
	 * Note that we do not use the ENUM SQL type because it is not supported by
	 * SQLite.
	 */
	class Database: public AbstractDatabase
	{
		public:
			// *** Data types
			class NotFoundException {};

			// *** Construction
			Database (const DatabaseInfo &dbInfo);
			virtual ~Database ();

			// *** Connection management
			bool open () { return defaultInterface.open (); }
			void close () { defaultInterface.close (); }
			QSqlError lastError () { return defaultInterface.lastError (); }


			// *** ORM
			// Template functions, instantiated for the relevant classes
			template<class T> QList<T> getObjects (QString condition="", QList<QVariant> conditionValues=QList<QVariant> ());
			template<class T> int countObjects ();
			template<class T> bool objectExists (dbId id);
			template<class T> T getObject (dbId id);
			template<class T> bool deleteObject (dbId id);
			template<class T> dbId createObject (T &object);
			template<class T> bool updateObject (const T &object);

			// *** Very specific
			virtual QStringList listStrings (const Query &query);
			virtual QList<Flight> getFlights (const QString &condition="", const QList<QVariant> &conditionValues=QList<QVariant> ());

		private:
			Interface::DefaultInterface defaultInterface;
	};
}

#endif
