#ifndef DATABASE_H_
#define DATABASE_H_

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>
#include <QSqlError>
#include <QObject>

#include "src/db/dbId.h"
#include "src/db/interface/Interface.h"
#include "src/db/event/Event.h"

class Flight;

namespace Db
{
	/**
	 * Methods for object-level access to the database
	 *
	 * This class implements the Greater ORM, which consists of:
	 *   - Object CRUDLEC (create, read, update, delete, list, exists, count)
	 *   - Selection frontends (e. g. getFlightsDate)
	 *   (- additional methods, like objectUsed)
	 *
	 * The CRUDLEC methods are templates which are instantiated for the
	 * relevant classes in the .cpp file.
	 *
	 * Database uses an Interface for access to the database. This class is not
	 * thread safe, but it may be used from any thread, provided that the
	 * Interface can (ThreadSafeInterface can, DefaultInterface can not).
	 *
	 * Note that we do not use the ENUM SQL type because it is not supported by
	 * SQLite.
	 */
	class Database: public QObject
	{
		Q_OBJECT;

		public:
			// *** Data types
			class NotFoundException {};

			// *** Construction
			Database (Interface::Interface &interface);
			virtual ~Database ();

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

		signals:
			void dbEvent (Event::Event event);

		private:
			Interface::Interface &interface;
	};
}

#endif
