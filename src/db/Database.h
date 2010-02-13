#ifndef _DATABASE_H
#define _DATABASE_H

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>

#include "src/db/dbTypes.h"

class LaunchType;
class Flight;
class DatabaseInfo;

/**
 * Methods for manipulating objects in the database (ORM).
 *
 * Defines template methods for retrieving, creating, counting, deleting,
 * creating and updating objects, which are instantiated for the relevant
 * classes in the .cpp file. Also provides some frontends for object selection
 * (like getFlightsDate).
 *
 * This class uses a QSqlDatabase, and as such is NOT thread safe. More
 * precisely, this class may only be used in the thread where it was created.
 * This is a QtSql restriction, see [1].
 *
 * [1] http://doc.trolltech.com/4.5/threads.html#threads-and-the-sql-module
 */
class Database
{
	public:
		class NotFoundException {};

		Database ();
		virtual ~Database ();

		// Connection management
		bool open (const DatabaseInfo &dbInfo);
		void close ();
		QSqlError lastError () const { return db.lastError (); }

		// Very generic
		QStringList listStrings (QString query);
		static QString selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty=false);

		// Database management (generic)
		bool addTable (QString name);
		bool addColumn (QString table, QString name, QString type);

		// Database management (specific)
		bool initializeDatabase ();

		// ORM
        // Template functions, instantiated for the relevant classes
        template<class T> QList<T> getObjects (QString condition="", QList<QVariant> conditionValues=QList<QVariant> ());
        template<class T> int countObjects ();
        template<class T> bool objectExists (db_id id);
        template<class T> T getObject (db_id id);
        template<class T> int deleteObject (db_id id);
        template<class T> db_id createObject (T &object);
        template<class T> int updateObject (const T &object);

        // Very specific
        QStringList listAirfields ();
        QStringList listAccountingNotes ();
        QStringList listClubs ();
        QStringList listPlaneTypes ();
        QList<Flight> getPreparedFlights ();
        QList<Flight> getFlightsDate (QDate date);


        // Database emulation (to be removed later)
        void addLaunchType (const LaunchType &launchType);

	protected:
    	// Data type names as in Rails (sk_web) (for MySQL)
    	static const QString dataTypeBinary;
    	static const QString dataTypeBoolean;
    	static const QString dataTypeDate;
    	static const QString dataTypeDatetime;
    	static const QString dataTypeDecimal;
    	static const QString dataTypeFloat;
    	static const QString dataTypeInteger;
    	static const QString dataTypeString;
    	static const QString dataTypeText;
    	static const QString dataTypeTime;
    	static const QString dataTypeTimestamp;
    	static const QString dataTypeCharacter; // Non-Rails
    	static const QString dataTypeId;

		QStringList listStrings (QSqlQuery query);

	private:
		QSqlDatabase db;

		// Legacy - the launch type list is not in the database
		QHash<db_id, LaunchType> launchTypes;
};

#endif
