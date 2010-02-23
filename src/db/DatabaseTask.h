/*
 * DatabaseTask.h
 *
 *  Created on: 23.02.2010
 *      Author: Martin Herrmann
 */

#ifndef DATABASETASK_H_
#define DATABASETASK_H_

#include <QMutex>
#include <QWaitCondition>

/**
 * A functor that is intended to run in the database thread. Other thread can
 * wait for the task to finish.
 */
class DatabaseTask
{
	public:
		DatabaseTask ();
		virtual ~DatabaseTask ();

		virtual void run ();

		virtual void wait ();

	protected:
		/** The method that does the actual work */
		virtual void execute ()=0;

	private:
		QMutex mutex;
		QWaitCondition finished;
		bool isFinished;
};

#endif
