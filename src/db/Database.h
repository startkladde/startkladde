#ifndef DATABASE_H_
#define DATABASE_H_

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>
#include <QSqlError>

#include "src/db/dbId.h"
#include "src/db/interface/Interface.h"

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
	 * This class is not thread safe, but it may be used from any thread.
	 *
	 * Note that we do not use the ENUM SQL type because it is not supported by
	 * SQLite.
	 */
	class Database
	{
		public:
			// *** Data types
			class NotFoundException {};

			// *** Construction
			Database (Interface::Interface &interface);
			virtual ~Database ();

			// *** Connection management
			// FIXME remove
			bool open () { return interface.open (); }
			void close () { interface.close (); }
			QSqlError lastError () { return interface.lastError (); }


			// *** ORM
			// Template functions, instantiated for the relevant classes
			template<class T> QList<T> getObjects (const Query &condition);
			template<class T> int countObjects (const Query &condition);
			template<class T> bool objectExists (dbId id);
			template<class T> T getObject (dbId id);
			template<class T> bool deleteObject (dbId id);
			template<class T> dbId createObject (T &object);
			template<class T> bool updateObject (const T &object);

			// We could use a default parameter for the corresponding methods
			// taking a Query&, but that would require all translation units
			// using this method to include Db::Query, which introduces
			// unnecessary dependencies.
			template<class T> QList<T> getObjects ();
			template<class T> int countObjects ();


			// *** Very specific
			virtual QStringList listLocations ();
			virtual QStringList listAccountingNotes ();
			virtual QStringList listClubs ();
			virtual QStringList listPlaneTypes ();
			virtual QList<Flight> getPreparedFlights ();
			virtual QList<Flight> getFlightsDate (QDate date);

		private:
			Interface::Interface &interface;
	};
}

#endif
