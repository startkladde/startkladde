#ifndef DATABASE_H_
#define DATABASE_H_

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>
#include <QSqlError>
#include <QSqlQuery>

#include "src/db/dbId.h"
#include "src/db/DatabaseInfo.h"
#include "src/db/DatabaseInterface.h" // TODO remove (to worker)

class Flight;

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
class Database
{
	public:
		// *** Data types
		class NotFoundException {};

    	// *** Construction
		Database ();
		virtual ~Database ();

		// *** Connection management
		bool open (const DatabaseInfo &dbInfo) { return databaseInterface.open (dbInfo); };
		void close () { databaseInterface.close (); }
		QSqlError lastError () { return databaseInterface.lastError (); }


		// *** ORM
        // Template functions, instantiated for the relevant classes
        template<class T> QList<T> getObjects (QString condition="", QList<QVariant> conditionValues=QList<QVariant> ());
        template<class T> int countObjects ();
        template<class T> bool objectExists (dbId id);
        template<class T> T getObject (dbId id);
        template<class T> int deleteObject (dbId id);
        template<class T> dbId createObject (T &object);
        template<class T> int updateObject (const T &object);

        // *** Very specific
        QStringList listLocations ();
        QStringList listAccountingNotes ();
        QStringList listClubs ();
        QStringList listPlaneTypes ();
        QList<Flight> getPreparedFlights ();
        QList<Flight> getFlightsDate (QDate date);

	private:
		DatabaseInterface databaseInterface;
};

#endif
