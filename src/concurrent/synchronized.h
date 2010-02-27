/*
 * synchronized.h
 *
 *  Created on: 27.02.2010
 *      Author: Martin Herrmann
 */

#ifndef SYNCHRONIZED_H_
#define SYNCHRONIZED_H_

#include <QMutex>
#include <QMutexLocker>

#define synchronized(mutex) for (Synchronizer _sync_ (&(mutex)); !_sync_.done; _sync_.done=true)

/**
 * A helper class to be used with the synchronized macro
 *
 * Use:
 *   QMutex mutex;
 *   synchronized (mutex) { ... }
 *
 * This is equivalent to, but more concise than:
 *   QMutex mutex;
 *   {
 *     QMutexLocker locker (&mutex);
 *     ...
 *   }
 *
 * The mutex will be unlocked even if the block returns or throws an exception.
 *
 * For single statements, this can be written as:
 *   synchronized (mutex) foo ();
 */
class Synchronizer: QMutexLocker
{
	public:
		Synchronizer (QMutex *mutex);
		virtual ~Synchronizer ();

		bool done;
};

#endif /* SYNCHRONIZED_H_ */
