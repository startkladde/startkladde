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

// *****

#include <iostream>
#include "src/db/DatabaseTask.h"
#include "src/db/Database.h"
#include "src/db/DatabaseInfo.h"

class WaitTask: public DatabaseTask
{
	virtual void execute ()
	{
		std::cout << "Waiting on thread " << QThread::currentThreadId () << std::endl;
		sleep (2);
		std::cout << "Finished on thread " << QThread::currentThreadId () << std::endl;
	}
};

template<class T> class GetObjectsTask: public DatabaseTask
{
	public:
		GetObjectsTask<T> (Database &database): database (database) {}

		virtual void execute ()
		{
			result=database.getObjects<T> ();
		}

		QList<T> result;

	private:
		Database &database;
};

class OpenTask: public DatabaseTask
{
	public:
		OpenTask (Database * &database, DatabaseInfo info):
			database (database), info (info) {}

		virtual void execute ()
		{
			// TODO leak
			database=new Database;
			database->open (info);
		}

	private:
		Database * &database;
		DatabaseInfo info;
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
class ThreadSafeDatabase
{
	public:
		ThreadSafeDatabase ();
		virtual ~ThreadSafeDatabase ();

		void open (DatabaseInfo info);

        template<class T> QList<T> getObjects ();

	private:
        DatabaseThread thread;
        Database *database;
};

#endif
