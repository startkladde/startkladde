/*
 * DefaultQThread.h
 *
 *  Created on: Aug 8, 2009
 *      Author: mherrman
 */

#ifndef DEFAULTQTHREAD_H_
#define DEFAULTQTHREAD_H_

// TODO reduce dependencies
#include <QThread>

class QObject;

class DefaultQThread: public QThread
{
	public:
		DefaultQThread (QObject *parent=NULL);
		virtual ~DefaultQThread ();

		static void sleep  (unsigned long secs ) { QThread:: sleep (secs ); }
		static void msleep (unsigned long msecs) { QThread::msleep (msecs); }
		static void usleep (unsigned long usecs) { QThread::usleep (usecs); }

	protected:
		virtual void run ();
};

#endif /* DEFAULTQTHREAD_H_ */
