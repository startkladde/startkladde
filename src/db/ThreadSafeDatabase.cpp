#include "ThreadSafeDatabase.h"

#include <iostream>

#include "src/db/Query.h"

// Required for template instantion
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"

namespace Db
{
	// ************************
	// ** ThreadSafeDatabase **
	// ************************

	ThreadSafeDatabase::ThreadSafeDatabase ():
		database (NULL)
	{
		thread.start ();
	}

	ThreadSafeDatabase::~ThreadSafeDatabase ()
	{
		std::cout << "Stoping thread and waiting" << std::endl;
		if (database) database->close (); // FIXME thread
		thread.exit (0);
		thread.wait ();
	}


	// ***************************
	// ** Connection management **
	// ***************************

	void ThreadSafeDatabase::open (DatabaseInfo info)
	{
		DatabaseOpenTask task (database, info);
		thread.runTask (task);
		task.wait ();
	}

	void ThreadSafeDatabase::close ()
	{
		// FIXME: strictly speaking, we're not supposed to do this on this thread,
		// and it might bite use in the rear
		database->close ();
	}

	QSqlError ThreadSafeDatabase::lastError ()
	{
		// FIXME: strictly speaking, we're not supposed to do this on this thread,
		// and it might bite use in the rear
		return database->lastError ();
	}


	// *********
	// ** ORM **
	// *********

	template<class T> QList<T> ThreadSafeDatabase::getObjects (const QString &condition, const QList<QVariant> &conditionValues)
	{
		DatabaseGetObjectsTask<T> task (database, condition, conditionValues);
		thread.runTaskAndWait (task);
		return task.result;
	}

	template<class T> int ThreadSafeDatabase::countObjects ()
	{
		DatabaseCountObjectsTask<T> task (database);
		thread.runTaskAndWait (task);
		return task.result;
	}

	template<class T> bool ThreadSafeDatabase::objectExists (dbId id)
	{
		DatabaseObjectExistsTask<T> task (database, id);
		thread.runTaskAndWait (task);
		return task.result;
	}

	template<class T> T ThreadSafeDatabase::getObject (dbId id)
	{
		DatabaseGetObjectTask<T> task (database, id);
		thread.runTaskAndWait (task);
		return task.result;
	}

	template<class T> bool ThreadSafeDatabase::deleteObject (dbId id)
	{
		DatabaseDeleteObjectTask<T> task (database, id);
		thread.runTaskAndWait (task);
		return task.result;
	}

	/**
	 * The id of the object is ignored and overwritten.
	 *
	 * @param object
	 * @return
	 */
	template<class T> dbId ThreadSafeDatabase::createObject (T &object)
	{
		DatabaseCreateObjectTask<T> task (database, object);
		thread.runTaskAndWait (task);
		return task.result;
	}

	template<class T> bool ThreadSafeDatabase::updateObject (const T &object)
	{
		DatabaseUpdateObjectTask<T> task (database, object);
		thread.runTaskAndWait (task);
		return task.result;
	}


	// ****************************
	// ** Template instantiation **
	// ****************************

	//template QList<Person> ThreadSafeDatabase::getObjects ();

	#define INSTANTIATE_TEMPLATES(T) \
		template QList<T> ThreadSafeDatabase::getObjects (const QString &condition, const QList<QVariant> &conditionValues); \
		template int      ThreadSafeDatabase::countObjects<T> (); \
		template bool     ThreadSafeDatabase::objectExists<T> (dbId id); \
		template T        ThreadSafeDatabase::getObject       (dbId id); \
		template bool     ThreadSafeDatabase::deleteObject<T> (dbId id); \
		template dbId     ThreadSafeDatabase::createObject    (T &object); \
		template bool     ThreadSafeDatabase::updateObject    (const T &object); \
		// Empty line

	INSTANTIATE_TEMPLATES (Person      )
	INSTANTIATE_TEMPLATES (Plane       )
	INSTANTIATE_TEMPLATES (Flight      )
	INSTANTIATE_TEMPLATES (LaunchMethod)

	#undef INSTANTIATE_TEMPLATES


	// *******************
	// ** Very specific **
	// *******************


	QStringList ThreadSafeDatabase::listStrings (const Query &query)
	{
		DatabaseListStringsTask task (database, query);
		thread.runTaskAndWait (task);
		return task.result;
	}

	QList<Flight> ThreadSafeDatabase::getFlights (const QString &condition, const QList<QVariant> &conditionValues)
	{
		return getObjects<Flight> (condition, conditionValues);
	}
}
