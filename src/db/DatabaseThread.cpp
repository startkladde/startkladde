#include "DatabaseThread.h"

#include "src/db/DatabaseTask.h"
#include "src/db/DatabaseWorker.h"

namespace Db
{
	DatabaseThread::DatabaseThread ():
		worker (NULL)
	{
	}

	DatabaseThread::~DatabaseThread ()
	{
	}

	void DatabaseThread::run ()
	{
		worker=new DatabaseWorker ();

		connect (
			this, SIGNAL (runTaskSignal (DatabaseTask *)),
			worker, SLOT (runTask (DatabaseTask *))
		);

		exec ();
	}

	/**
	 * Sends a runTaskSignal with the given task to the database thread.
	 *
	 * @param task the task to execute. The task may not be destroyed until it has
	 *             finished!
	 */
	void DatabaseThread::runTask (DatabaseTask &task)
	{
		emit runTaskSignal (&task);
	}

	/**
	 * Runs the task and waits for it to finish.
	 *
	 * @param task the task to execute. The task can be destroyed after this method
	 *             returns.
	 */
	void DatabaseThread::runTaskAndWait (DatabaseTask &task)
	{
		emit runTaskSignal (&task);
		task.wait ();
	}
}
