#include "DbWorker.h"

#include <iostream>

#include "src/db/Database.h"

#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"

// FIXME: what happens if an exception is not caught?
// TODO: this requires Qt 4.4 (Hardy has 4.3)

/*
 * Now, the implementation of this here worker class is a bit more complicated
 * than that of other worker classes, due to the fact that we need template
 * methods (e. g. addObject), but cannot have template signals/slots. Thus, we
 * have to resort to functors (class Task).
 */

namespace Db
{
	// ***********
	// ** Tasks **
	// ***********

	template<class T> class CreateObjectTask: public DbWorker::Task
	{
		public:
			CreateObjectTask (Returner<dbId> *returner, T &object):
				returner (returner), object (object)
			{
			}

			Returner<dbId> *returner;
			T &object;

			virtual void run (Database &db, OperationMonitor *monitor)
			{
				OperationMonitorInterface interface=monitor->interface ();
				returnOrException (returner, db.createObject (object));
			}
	};

	template<class T> class DeleteObjectTask: public DbWorker::Task
	{
		public:
			DeleteObjectTask (Returner<int> *returner, dbId id):
				returner (returner), id (id)
			{
			}

			Returner<int> *returner;
			dbId id;

			virtual void run (Database &db, OperationMonitor *monitor)
			{
				OperationMonitorInterface interface=monitor->interface ();
				returnOrException (returner, db.deleteObject<T> (id));
			}
	};

	template<class T> class UpdateObjectTask: public DbWorker::Task
	{
		public:
			UpdateObjectTask (Returner<int> *returner, const T &object):
				returner (returner), object (object)
			{
			}

			Returner<int> *returner;
			const T &object;

			virtual void run (Database &db, OperationMonitor *monitor)
			{
				OperationMonitorInterface interface=monitor->interface ();
				returnOrException (returner, db.updateObject (object));
			}
	};

	template<class T> class ObjectUsedTask: public DbWorker::Task
	{
		public:
			ObjectUsedTask (Returner<bool> *returner, dbId id):
				returner (returner), id (id)
			{
			}

			Returner<bool> *returner;
			const dbId id;

			virtual void run (Database &db, OperationMonitor *monitor)
			{
				OperationMonitorInterface interface=monitor->interface ();
				returnOrException (returner, db.objectUsed<T> (id));
			}
	};



	// ******************
	// ** Construction **
	// ******************

	DbWorker::DbWorker (Database &db):
		db (db)
	{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
		CONNECT (executeAndDeleteTask (OperationMonitor *, Task *));
#undef CONNECT

		moveToThread (&thread);
		thread.start ();
	}

	DbWorker::~DbWorker ()
	{
		thread.quit ();

		std::cout << "Waiting for ORM worker thread to terminate...";
		std::cout.flush ();

		if (thread.wait (1000))
			std::cout << "OK" << std::endl;
		else
			std::cout << "Timeout" << std::endl;
	}

	// ***********************
	// ** Front-end methods **
	// ***********************

	template<class T> void DbWorker::createObject (Returner<dbId> &returner, OperationMonitor &monitor, T &object)
	{
		executeAndDeleteTask (&monitor, new CreateObjectTask<T> (&returner, object));
	}

	template<class T> void DbWorker::deleteObject (Returner<int> &returner, OperationMonitor &monitor, dbId id)
	{
		executeAndDeleteTask (&monitor, new DeleteObjectTask<T> (&returner, id));
	}

	template<class T> void DbWorker::updateObject (Returner<int> &returner, OperationMonitor &monitor, const T &object)
	{
		executeAndDeleteTask (&monitor, new UpdateObjectTask<T> (&returner, object));
	}

	template<class T> void DbWorker::objectUsed (Returner<bool> &returner, OperationMonitor &monitor, dbId id)
	{
		executeAndDeleteTask (&monitor, new ObjectUsedTask<T> (&returner, id));
	}

	void DbWorker::executeAndDeleteTask (OperationMonitor *monitor, DbWorker::Task *task)
	{
		emit sig_executeAndDeleteTask (monitor, task);
	}


	// ********************
	// ** Back-end slots **
	// ********************

	void DbWorker::slot_executeAndDeleteTask (OperationMonitor *monitor, DbWorker::Task *task)
	{
		task->run (db, monitor);
		delete task;
	}


#	define INSTANTIATE_TEMPLATES(T) \
		template class CreateObjectTask<T>; \
		template void DbWorker::createObject<T> (Returner<dbId> &returner, OperationMonitor &monitor, T &object); \
		template void DbWorker::deleteObject<T> (Returner<int > &returner, OperationMonitor &monitor, dbId id); \
		template void DbWorker::updateObject<T> (Returner<int > &returner, OperationMonitor &monitor, const T &object); \
		template void DbWorker::objectUsed  <T> (Returner<bool> &returner, OperationMonitor &monitor, dbId id); \
		// Empty line

	INSTANTIATE_TEMPLATES (Person      )
	INSTANTIATE_TEMPLATES (Plane       )
	INSTANTIATE_TEMPLATES (Flight      )
	INSTANTIATE_TEMPLATES (LaunchMethod)

#	undef INSTANTIATE_TEMPLATES

}

