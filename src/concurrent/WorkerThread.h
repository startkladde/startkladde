/*
 * WorkerThread.h
 *
 *  Created on: Aug 9, 2009
 *      Author: mherrman
 */

#ifndef WORKERTHREAD_H_
#define WORKERTHREAD_H_

// TODO reduce dependencies
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QList>

class Task;

class WorkerThread: public QThread
{
	public:
		WorkerThread (QString name);
		virtual ~WorkerThread ();

		void addTask (Task *task);

		void stop (bool waitUntilFinished);

	protected:
		virtual void run ();

	private:
		QString name;

		bool stopped;

		QList<Task *> tasks; // All accesses to this list must be protected by listMutex.
		QMutex listMutex;
		QWaitCondition listChanged; // Wakes on this condition must be protected by listMutex.
};

#endif /* WORKERTHREAD_H_ */
