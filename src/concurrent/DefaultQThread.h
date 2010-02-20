#ifndef _defaultQThread_h
#define _defaultQThread_h

#include <QThread>

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
