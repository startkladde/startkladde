/*
 * DatabaseThread.h
 *
 *  Created on: 23.02.2010
 *      Author: Martin Herrmann
 */

#ifndef DATABASETHREAD_H_
#define DATABASETHREAD_H_

#include <QThread>

class DatabaseTask;
class DatabaseWorker;

/**
 * Runs a worker in a background thread and delivers signals to it
 *
 * The methods of this class can be called from any thread. A corresponding
 * signal is delivered to the worker in the background thread.
 *
 * This class is thread safe.
 */
class DatabaseThread: public QThread
{
	Q_OBJECT;

	public:
		DatabaseThread ();
		virtual ~DatabaseThread ();

		void runTask (DatabaseTask &task);

	signals:
		void runTaskSignal (DatabaseTask *task);

	protected:
		virtual void run ();

	private:
		DatabaseWorker *worker;

};

#endif
