/*
 * ThreadSafeDatabase.h
 *
 *  Created on: 23.02.2010
 *      Author: Martin Herrmann
 */

#ifndef THREADSAFEDATABASE_H_
#define THREADSAFEDATABASE_H_

#include "src/db/dbId.h"
#include "src/db/DatabaseThread.h"
#include "src/db/AbstractDatabase.h"

// *****

#include <iostream>
#include "src/db/DatabaseTask.h"
#include "src/db/Database.h"
#include "src/db/DatabaseInfo.h"
#include "src/db/interface/DefaultInterface.h"

//class WaitTask: public DatabaseTask
//{
//	virtual void execute ()
//	{
//		std::cout << "Waiting on thread " << QThread::currentThreadId () << std::endl;
//		sleep (2);
//		std::cout << "Finished on thread " << QThread::currentThreadId () << std::endl;
//	}
//};

namespace Db
{
	class DatabaseOpenTask: public DatabaseTask
	{
		public:
			DatabaseOpenTask (Database * &database, DatabaseInfo info):
				DatabaseTask (database), info (info) {}

			virtual void execute ()
			{
				// TODO leak
				Interface::Interface *interface=new Interface::DefaultInterface (info);
				database=new Db::Database (*interface);
				database->open ();
			}

		private:
			DatabaseInfo info;
	};

	template<class T> class DatabaseGetObjectsTask: public DatabaseTask
	{
		public:
			DatabaseGetObjectsTask<T> (Database * &database, const QString &condition="", const QList<QVariant> &conditionValues=QList<QVariant> ()):
				DatabaseTask (database), condition (condition), conditionValues (conditionValues)
				{}

			virtual void execute ()
			{
				result=database->getObjects<T> (condition, conditionValues);
			}

			QList<T> result;

		private:
			const QString &condition;
			const QList<QVariant> &conditionValues;
	};

	template<class T> class DatabaseCountObjectsTask: public DatabaseTask
	{
		public:
			DatabaseCountObjectsTask<T> (Database * &database):
				DatabaseTask (database) {}

			virtual void execute ()
			{
				result=database->countObjects<T> ();
			}

			int result;
	};

	template<class T> class DatabaseObjectExistsTask: public DatabaseTask
	{
		public:
			DatabaseObjectExistsTask<T> (Database * &database, dbId id):
				DatabaseTask (database), id (id) {}

			virtual void execute ()
			{
				result=database->objectExists<T> (id);
			}

			bool result;

		private:
			dbId id;
	};

	template<class T> class DatabaseGetObjectTask: public DatabaseTask
	{
		public:
			DatabaseGetObjectTask<T> (Database * &database, dbId id):
				DatabaseTask (database), id (id) {}

			virtual void execute ()
			{
				result=database->getObject<T> (id);
			}

			T result;

		private:
			dbId id;
	};

	template<class T> class DatabaseDeleteObjectTask: public DatabaseTask
	{
		public:
			DatabaseDeleteObjectTask<T> (Database * &database, dbId id):
				DatabaseTask (database), result (false), id (id) {}

			virtual void execute ()
			{
				result=database->deleteObject<T> (id);
			}

			bool result;

		private:
			dbId id;
	};

	template<class T> class DatabaseCreateObjectTask: public DatabaseTask
	{
		public:
			DatabaseCreateObjectTask<T> (Database * &database, T &object):
				DatabaseTask (database), result (false), object (object) {}

			virtual void execute ()
			{
				result=database->createObject<T> (object);
			}

			dbId result;

		private:
			T &object;
	};

	template<class T> class DatabaseUpdateObjectTask: public DatabaseTask
	{
		public:
			DatabaseUpdateObjectTask<T> (Database * &database, const T &object):
				DatabaseTask (database), result (false), object (object) {}

			virtual void execute ()
			{
				result=database->updateObject<T> (object);
			}

			bool result;

		private:
			const T &object;
	};

	class DatabaseListStringsTask: public DatabaseTask
	{
		public:
			DatabaseListStringsTask (Database * &database, const Query &query):
				DatabaseTask (database), query (query) {}

			virtual void execute ()
			{
				result=database->listStrings (query);
			}

			QStringList result;

		private:
			const Query &query;
	};

	// *****

	/**
	 * A thread safe alternative to Database
	 *
	 * Thread safety is achieved by keeping a Database instance in a background
	 * thread and performing all accesses in the background thread. All calls are
	 * still blocking, that is, they only return after the operation finished in
	 * the background thread.
	 */
	class ThreadSafeDatabase: public AbstractDatabase
	{
		public:
			// *** Construction
			ThreadSafeDatabase ();
			virtual ~ThreadSafeDatabase ();

			// *** Connection management
			void open (DatabaseInfo info);
			void close ();
			QSqlError lastError ();

			// *** ORM
			// Template functions, instantiated for the relevant classes
			template<class T> QList<T> getObjects (const QString &condition="", const QList<QVariant> &conditionValues=QList<QVariant> ());
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
			DatabaseThread thread;
			Database *database;
	};
}

#endif
