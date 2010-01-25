#ifndef _DATABASE_H
#define _DATABASE_H

#include <QString>
#include <QList>
#include <QtSql>

#include "src/db/dbTypes.h"

class Person;

class Database
{
	public:
		class NotFoundException {};

		Database ();
		virtual ~Database ();

		bool open () { return db.open (); }
		QSqlError lastError () const { return db.lastError (); }

        // Template functions, instantiated for the relevant classes
        template<class T> QList<T> getObjects ();
        template<class T> int countObjects ();
        template<class T> bool objectExists (db_id id);
        template<class T> T getObject (db_id id);
        template<class T> int deleteObject (db_id id);
        template<class T> db_id createObject (T &object);
        template<class T> int updateObject (const T &object);

	private:
		QSqlDatabase db;

};

#endif
