/*
 * DataStorageWorker.h
 *
 *  Created on: Aug 16, 2009
 *      Author: mherrman
 */

#ifndef DATASTORAGEWORKER_H_
#define DATASTORAGEWORKER_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class DataStorage;
class Task;

class DataStorageWorker: public QThread
{
	friend class DataStorage;

	private:
		DataStorageWorker (DataStorage &dataStorage, int pingInterval);
		virtual ~DataStorageWorker ();

		void scheduleConnect ();
		void scheduleDisconnect ();
		void addTask (Task *task);

		void stop (bool waitUntilFinished);

	protected:
		virtual void run ();

	private:
		DataStorage &dataStorage;
		int pingInterval;

		QMutex mutex;
		// Wakes on this condition must be protected by mutex with the write to
		// the variable that indicates the work to do. Otherwise, this might
		// happen before the thread enters the wait state.
		QWaitCondition workToDo;

		bool connectScheduled, disconnectScheduled;

		bool stopped;
		QList<Task *> tasks; // All accesses to this list must be protected by mutex
};

#endif /* DATASTORAGEWORKER_H_ */
