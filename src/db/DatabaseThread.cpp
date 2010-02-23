#include "DatabaseThread.h"

#include "src/db/DatabaseTask.h"
#include "src/db/DatabaseWorker.h"

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
 * Emits a runTaskSignal which will be received by the database thread.
 *
 * @param task the task to execute. The task may not be destroyed until it has
 *             finished!
 */
void DatabaseThread::runTask (DatabaseTask &task)
{
	emit runTaskSignal (&task);
}
