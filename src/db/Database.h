#ifndef _DATABASE_H
#define _DATABASE_H

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>

#include "src/db/dbTypes.h"

class LaunchType;

class Database
{
	public:
		class NotFoundException {};

		Database ();
		virtual ~Database ();

		// Connection management
		bool open (QString server, int port, QString username, QString password, QString database);
		void close ();
		QSqlError lastError () const { return db.lastError (); }

		// Very generic
		QStringList listStrings (QString query);
		static QString selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty=false);
		static QString selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty=false);

		// ORM
        // Template functions, instantiated for the relevant classes
        template<class T> QList<T> getObjects ();
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


        // Database emulation (to be removed)
        void addLaunchType (const LaunchType &launchType);

	protected:
		QStringList listStrings (QSqlQuery query);

	private:
		QSqlDatabase db;

		// Legacy - the launch type list is not in the database
		QHash<db_id, LaunchType> launchTypes;
};

#endif
