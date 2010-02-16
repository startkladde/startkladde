#ifndef _DATABASE_H
#define _DATABASE_H

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>
#include <QSqlError>
#include <QSqlQuery>

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
		// *** Data types
		class NotFoundException {};
		class QueryFailedException
		{
			public:
				QueryFailedException (QSqlQuery query): query (query) {}
				QSqlQuery query;
		};

    	// *** Constants

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

    	// *** Construction
		Database ();
		virtual ~Database ();

		// *** Connection management
		bool open (const DatabaseInfo &dbInfo);
		void close ();
		QSqlError lastError () const { return db.lastError (); }

		// *** Transactions
		bool transaction () { return db.transaction (); }
		bool commit      () { return db.commit      (); }
		bool rollback    () { return db.rollback    (); }

		// *** Queries
		QSqlQuery prepareQuery (QString queryString);
		QSqlQuery &executeQuery (QSqlQuery &query);
		QSqlQuery executeQuery (QString queryString);


		// *** Very generic
		QStringList listStrings (QString queryString);
		static QString selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty=false);
		bool queryHasResult (QSqlQuery &query);
		bool queryHasResult (QString queryString);


		// *** Shema manipulation
		void createTable (QString name);
		void createTableLike (QString like, QString name);
		void dropTable (QString name);
		void renameTable (QString oldName, QString newName);
		void addColumn (QString table, QString name, QString type, bool skipIfExists=false);
		void changeColumnType (QString table, QString name, QString type);
		void dropColumn (QString table, QString name, bool skipIfNotExists=false);
		bool tableExists (QString name);
		bool columnExists (QString table, QString name);

		// *** ORM
        // Template functions, instantiated for the relevant classes
        template<class T> QList<T> getObjects (QString condition="", QList<QVariant> conditionValues=QList<QVariant> ());
        template<class T> int countObjects ();
        template<class T> bool objectExists (db_id id);
        template<class T> T getObject (db_id id);
        template<class T> int deleteObject (db_id id);
        template<class T> db_id createObject (T &object);
        template<class T> int updateObject (const T &object);

        // *** Very specific
        QStringList listAirfields ();
        QStringList listAccountingNotes ();
        QStringList listClubs ();
        QStringList listPlaneTypes ();
        QList<Flight> getPreparedFlights ();
        QList<Flight> getFlightsDate (QDate date);


        // *** Database emulation (to be removed later)
        void addLaunchType (const LaunchType &launchType);

	protected:
		QStringList listStrings (QSqlQuery query);

	private:
		QSqlDatabase db;

		// Legacy - the launch type list is not in the database
		QHash<db_id, LaunchType> launchTypes;
};

#endif
