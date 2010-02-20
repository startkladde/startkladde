#include "WorkerThread.h"

#include <iostream>

#include <QMutexLocker>

#include "src/concurrent/task/Task.h"
#include "src/text.h"

WorkerThread::WorkerThread (QString name):
	name (name), stopped (false)
{
}

WorkerThread::~WorkerThread ()
{
}

// TODO: introduce a maximum wait time (note the QThread may not be deleted
// while the thread is running)
// TODO: cancel the current task.
void WorkerThread::stop (bool waitUntilFinished)
{
	// TODO rename the listMutex. We need to do this because the mutex must be
	// locked while waking.
	QMutexLocker listLocker (&listMutex);
	stopped=true;
	listChanged.wakeAll ();
	listLocker.unlock ();


	// TODO introduce a max wait time
	if (waitUntilFinished)
	{
		if (!wait (200))
		{
			std::cout << name << ": waiting for the worker thread to finish" << std::endl;
			wait ();
			std::cout << name << ": worker thread finished" << std::endl;
		}
	}
}

/**
 * Adds a task to the list of tasks to be performed. This method will typically
 * be called from another thread.
 *
 * Note that after this method returns, the task has not necessarily been
 * started, because (a) there may be other tasks still running, and (b) even
 * if there are not, the task is started asynchronously by the worker thread.
 *
 * The task may not been deleted until after is has ended (isEnded
 * returns true).
 *
 * The task is not moved to the worker thread. Even if it was, you still
 * couldn't call deleteLater because the worker thread does not have an evenet
 * loop.
 *
 * @param task the task to addObject to the list. No ownership is taken.
 */
void WorkerThread::addTask (Task *task)
{
	// Lock the list. If the thread is currently accessing the list, this will
	// block until the access to the list is finished.
	QMutexLocker listLocker (&listMutex);

	std::cout << QString ("%1 scheduling task \"%2\"").arg (name).arg (task->toString ()) << std::endl;

	// Add the thread to the list.
	tasks.append (task);

	// Wake up the thread (there should be only one) waiting for list changes.
	// This must be done while listMutex is locked, because otherwise this
	// might happen between run() finding that the task list is empty and
	// entering the wait state, thus never being woken up.
	listChanged.wakeAll ();

	// listMutex will automatically unlocked
}

void WorkerThread::run ()
{
	// Lock the list for accessing it
	QMutexLocker listLocker (&listMutex);

	while (!stopped) // Access to stopped: OK, because the mutex is locked
	{
		while (!tasks.empty () && !stopped)
		{
			// Take the first task from the list
			Task *task=tasks.takeFirst ();

			// Unlock the listMutex while running the task. This is important as
			// otherwise addTask would block.
			listLocker.unlock ();

			QString taskText=task->toString ();
			std::cout << QString ("%1 starting task \"%2\"").arg (name).arg (taskText) << std::endl;
			task->start ();
			std::cout << QString ("Task \"%1\" %2").arg (taskText).arg (task->isCompleted ()?"completed":"aborted") << std::endl;

			listLocker.relock ();
		}

		if (stopped)
			return;

		// Wait for notification on listChanged. Note that the transition from
		// the locked state to the waiting ist atomic. This is important
		// because otherwise, a task might be added between unlocking the mutex
		// and entering the wait state, so the wake notification would be
		// missed.
		listChanged.wait (listLocker.mutex ());
	}
}
