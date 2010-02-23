#include "ThreadSafeDatabase.h"

#include <iostream>

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
	if (database) database->close ();
	thread.exit (0);
	thread.wait ();
}

//void ThreadSafeDatabase::wait ()
//{
//	WaitTask t;
//
//	thread.runTask (t);
//	std::cout << "Waiting for task on thread " << QThread::currentThreadId () << std::endl;
//	t.wait ();
//}

void ThreadSafeDatabase::open (DatabaseInfo info)
{
	OpenTask task (database, info);
	thread.runTask (task);
	task.wait ();
}

template<class T> QList<T> ThreadSafeDatabase::getObjects ()
{
	GetObjectsTask<T> task (*database);
	thread.runTask (task);
	task.wait ();

	return task.result;
}

//#define INSTANTIATE_TEMPLATES(Class) \
//	template QList<Class> Database::getObjects (QString condition, QList<QVariant> conditionValues); \
//	template int          Database::countObjects<Class> (); \
//	template bool         Database::objectExists<Class> (dbId id); \
//	template Class        Database::getObject           (dbId id); \
//	template int          Database::deleteObject<Class> (dbId id); \
//	template dbId         Database::createObject        (Class &object); \
//	template int          Database::updateObject        (const Class &object); \
//	// Empty line

#include "src/model/Person.h"

template QList<Person> ThreadSafeDatabase::getObjects ();

//#define INSTANTIATE_TEMPLATES(Class) \
//	template QList<Class> ThreadSafeDatabase::getObjects (); \
//	// Empty line
//
//INSTANTIATE_TEMPLATES (Person      )
//INSTANTIATE_TEMPLATES (Plane       )
//INSTANTIATE_TEMPLATES (Flight      )
//INSTANTIATE_TEMPLATES (LaunchMethod)
//
//#undef INSTANTIATE_TEMPLATES
