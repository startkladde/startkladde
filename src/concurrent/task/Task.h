/*
 * Task.h
 *
 *  Created on: Aug 8, 2009
 *      Author: mherrman
 */

#ifndef TASK_H_
#define TASK_H_

#include <QObject>
#include <QMutex>

// TODO reduce dependencies
#include "src/concurrent/monitor/OperationMonitor.h"

/*
 * TODO: we need a way of specifying the maximum progress, here and in
 *   OperationMonitor, because it may not be known in the beginning
 *   But we also need to specify several operations.
 * TODO: we need a way to discard a canceled task so it waits for ending
 *   in the background
 * TODO: we should have a Task::Result or similar for returning completed,
 *   succeeded and message.
 *
 */

/**
 * A task which can be executed and canceled.
 *
 * The Task will emit signals when it is started, when the progress changed,
 * when it completes, when it ends and when it is canceled.
 *
 * This class is thread safe.
 *
 * Notes for implementations:
 *   - do not emit the started, ended or canceled signals from the run
 *     implementation - they are emitted by start at the appropriate time
 *   - implement toString to return a textual description of the task
 *   - implement run to return true if the task finished and false if it was
 *     canceled
 */
class Task: public QObject, public OperationMonitor
{
	Q_OBJECT

	public:
		Task ();
		virtual ~Task ();

		void start ();
		void cancel ();

		bool isCanceled () const;
		bool isRunning () const;
		bool isCompleted () const;
		bool isEnded () const;

		virtual QString toString () const=0;

		virtual bool getSuccess () const { QMutexLocker lock (&mutex); return success; }
		virtual QString getMessage () const { QMutexLocker lock (&mutex); return message; }


	/*private*/ signals:
		/** Emitted when the task is started */
		void started ();

		/** Emitted after the task stop running, either because it was
		 * canceled or it completed */
		void ended ();

		/** Emitted after the task has been canceled. It may still be
		 * running. */
		void canceled ();


	signals:
		/** Emitted while the task is running */
		void progressChanged (int progress, int maxProgress);

		/** Emitted while the task is running */
		void statusChanged (QString status);

	protected:
		void progress (int progress, int maxProgress);
		void status (QString status);

		virtual void setSuccess (bool success) { QMutexLocker lock (&mutex); this->success=success; }
		virtual void setMessage (QString message) { QMutexLocker lock (&mutex); this->message=message; }


	protected:
		/** Runs the task on the current thread. This method is run unsynchronized.
		 * @return true on completed, false on canceled (regardless of success)
		 */
		virtual bool run ()=0;
		void taskCompleted ();


	private:
		bool _canceled;
		bool _running;
		bool _ended;
		bool _completed;

		bool success;
		QString message;

		mutable QMutex mutex;
		bool autoDelete;
};

#endif /* TASK_H_ */
