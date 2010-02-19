/*
 * Task.cpp
 *
 *  Created on: Aug 8, 2009
 *      Author: mherrman
 */

#include "Task.h"

#include <iostream>

#include <QMutexLocker>
#include <QApplication>

#include "src/text.h"

/* Notes:
 *   - signals may only be emitted while the mutex is unlocked, so properties
 *     can be queried, and after updating the properties
 *   - we need the run implementation to return whether the task has been
 *     canceled rather than looking at the canceled flag after run returns,
 *     because a task may finish even if it is canceled, or it may be canceled
 *     after finishing
 */

Task::Task ():
	success (false),
	_canceled (false),
	_running (false),
	_ended (false),
	_completed (false)
{
}

Task::~Task ()
{
	// We cannot call toString here as it is virtual
	if (isRunning ())
		std::cerr << "Error: Task deleted while still running" << std::endl;
}

/**
 * Starts the task on the current thread. If the task is already running, do
 * nothing. A task may be started again, even if it was canceled before.
 */
void Task::start ()
{
	QMutexLocker mutexLocker (&mutex);
	{
		// If already running, return.
		// TODO signal error
		if (_running) return;

		// If it was canceled, return. A task may be canceled after it has
		// been scheduled, but before it is started.
		if (_canceled)
		{
			_ended=true;

			mutexLocker.unlock ();
			emit ended ();
			return;
		}

		// Set the state to running (atomically with the running check above)
		_running=true;
		_canceled=false;
		_completed=false;
		_ended=false;
	}
	mutexLocker.unlock ();

	emit started ();

	// Run the task (without locking the mutex, so the state can be queried)
	bool completedResult=run ();

	mutexLocker.relock ();
	{
		// Set the state to not running
		_running=false;
		_ended=true;
		_completed=completedResult;
	}
	mutexLocker.unlock ();

	emit ended ();
}

/**
 * Schedule the task to cancel. You have to wait for isRunning to become false
 * (or the ended signal) before you can delete the Task.
 *
 * @see isRunning
 */
void Task::cancel ()
{
	QMutexLocker locker (&mutex);
	_canceled=true;
	locker.unlock ();
	emit canceled ();
}

/**
 * Whether cancel() has been called on the task. The task may still be running,
 * or it may have completed after cancel() has been called.
 *
 * @see isRunning
 */
bool Task::isCanceled () const
{
	QMutexLocker locker (&mutex);
	return _canceled;
}

/**
 * Whether the task is running at the moment. This will become false after the
 * task either completes or after the task is canceled.
 *
 * @see isCompleted
 */
bool Task::isRunning () const
{
	QMutexLocker locker (&mutex);
	return _running;
}

/**
 * Whether the task completed. If the task is canceled after completing, this
 * will still be true.
 */
bool Task::isCompleted () const
{
	QMutexLocker locker (&mutex);
	return _completed;
}


/**
 * Whether the task ended. This can be used to distinguish a task that has not
 * been started from task that ended.
 */
bool Task::isEnded () const
{
	QMutexLocker locker (&mutex);
	return _ended;
}

// ******************************
// ** OperationMonitor methods **
// ******************************

void Task::progress (int progress, int maxProgress)
{
	emit progressChanged (progress, maxProgress);
}

void Task::status (QString status)
{
	emit statusChanged (status);
}
