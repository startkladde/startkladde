#include "DatabaseTask.h"

#include <QMutexLocker>

DatabaseTask::DatabaseTask (Database * &database):
	database (database), isFinished (false)
{
}

DatabaseTask::~DatabaseTask ()
{
}

/** Runs the task and notifies thread waiting in #wait */
void DatabaseTask::run ()
{
	// Lock the mutex outside of the try block because in case of an exception,
	// the catch block must be executed locked.
	QMutexLocker locker (&mutex);

	try
	{
		// Execute the task
		execute ();

		// Set the finished flag and wake all thread waiting for this task.
		// This must be synchronized or it could happen between #wait checking
		// the finished flag and waiting on the finished wait condition.
		isFinished=true;
		finished.wakeAll ();
	}
	catch (...)
	{
		// An exception was thrown. Signal the task finished and rethrow the
		// exception.
		isFinished=true;
		finished.wakeAll ();

		throw;
	}
}

/**
 * Waits for the task to finish in another thread
 *
 * If the task already finished, this method returns right away.
 */
void DatabaseTask::wait ()
{
	// Lock the mutex
	QMutexLocker locker (&mutex);

	// If the task starts (and potentially finishes) before entering this
	// method, the task will have finished by the time we get the mutex. In
	// this case, we return right away.
	// This check and the call to wait must be synchronized or the wakeup call
	// might happen between the check and the wait and the wait would never
	// return.
	if (isFinished) return;

	// The task has not started yet. Let it start and wait until it signals
	// that it finished.
	finished.wait (&mutex);
}
