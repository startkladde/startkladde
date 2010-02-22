/*
 * DataStorageWorker.cpp
 *
 *  Created on: Aug 16, 2009
 *      Author: mherrman
 */

#include "DataStorageWorker.h"

#include <iostream>

#include <QMutexLocker>

//#include "src/text.h"
#include "src/db/dataStorage/DataStorage.h"
#include "src/concurrent/task/Task.h"
#include "src/util/qString.h"

DataStorageWorker::DataStorageWorker (DataStorage &dataStorage, int pingInterval) :
	dataStorage (dataStorage), pingInterval (pingInterval), connectScheduled (false), disconnectScheduled (false),
			stopped (false)
{
}

DataStorageWorker::~DataStorageWorker ()
{
}

// TODO: introduce a maximum wait time (note the QThread may not be deleted
// while the thread is running)
// TODO: cancel the current task.
void DataStorageWorker::stop (bool waitUntilFinished)
{
	// TODO rename the listMutex. We need to do this because the mutex must be
	// locked while waking.
	QMutexLocker locker (&mutex);
	stopped=true;
	workToDo.wakeAll ();
	locker.unlock ();

	if (waitUntilFinished)
	{
		if (!wait (200))
		{
			std::cout << "Waiting for the data storage worker thread to finish" << std::endl;
			wait ();
			std::cout << "Data storage worker thread finished" << std::endl;
		}
	}
}

void DataStorageWorker::scheduleConnect ()
{
	QMutexLocker locker (&mutex);
	connectScheduled=true;
	workToDo.wakeAll ();
}

void DataStorageWorker::scheduleDisconnect ()
{
	QMutexLocker locker (&mutex);
	disconnectScheduled=true;
	workToDo.wakeAll ();
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
void DataStorageWorker::addTask (Task *task)
{
	// Lock the list. If the thread is currently accessing the list, this will
	// block until the access to the list is finished.
	QMutexLocker locker (&mutex);

	std::cout << QString ("Data storage worker scheduling task \"%1\"").arg (task->toString ()) << std::endl;

	// Add the thread to the list.
	tasks.append (task);

	// Wake up the thread (there should be only one) waiting for list changes.
	// This must be done while listMutex is locked, because otherwise this
	// might happen between run() finding that the task list is empty and
	// entering the wait state, thus never being woken up.
	workToDo.wakeAll ();

	// mutex will automatically unlocked
}

void DataStorageWorker::run ()
{
	// Protect accesses to member variables
	QMutexLocker locker (&mutex);

	while (!stopped)
	{
		if (stopped) return;

		if (connectScheduled)
		{
			locker.unlock ();
			std::cout << "DataStorageWorker: connecting" << std::endl;
			dataStorage.doConnect ();
			locker.relock ();
			connectScheduled=false;
		}

		while (!tasks.empty () && !stopped)
		{
			// Take the first task from the list
			Task *task=tasks.takeFirst ();

			// Unlock the listMutex while running the task. This is important as
			// otherwise addTask would block.
			locker.unlock ();

			QString taskText=task->toString ();
			std::cout << QString ("Data storage worker starting task \"%1\"").arg (taskText) << std::endl;
			task->start ();
			std::cout << QString ("Task \"%1\" %2").arg (taskText).arg (task->isCompleted () ? "completed" : "aborted")
					<< std::endl;

			locker.relock ();
		}

		if (disconnectScheduled)
		{
			locker.unlock ();
			std::cout << "DataStorageWorker: disconnecting" << std::endl;
			dataStorage.doDisconnect ();
			locker.relock ();
			disconnectScheduled=false;
		}

		// Wait for notification on workToDo. Note that the transition from
		// the locked state to the waiting ist atomic. This is important
		// because otherwise, a task might be added between unlocking the mutex
		// and entering the wait state, so the wake notification would be
		// missed.
		bool waitResult=workToDo.wait (locker.mutex (), pingInterval);

		locker.unlock ();

		if (!waitResult)
		{
			// Wait for useful work timed out - ping the database
			if (dataStorage.isConnectionEstablished ())
			{
//				std::cout << "Ping...";
//				std::cout.flush ();

				bool alive=dataStorage.ping ();

				if (alive)
				{
//					std::cout << "Pong" << std::endl;
					dataStorage.setState (DataStorage::stateConnected);
				}
				else
				{
//					std::cout << std::endl;
					dataStorage.setState (DataStorage::stateLost);
				}


			}
		}

		locker.relock ();
	}
}
