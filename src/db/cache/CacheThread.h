/*
 * CacheThread.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHETHREAD_H_
#define CACHETHREAD_H_

#include <QThread>
#include <QDate>

template<typename T> class Returner;
class OperationMonitor;

namespace Db
{
	namespace Cache
	{
		class Cache;

		/**
		 * A background worker to perform cache related work in the background
		 *
		 * All methods return immediately. The result of the operation is
		 * returned using a Returner. The operation can be monitored and
		 * canceled (if supported by the operation) through an
		 * OperationMonitor. returnedValue or wait must be called on the
		 * returner after calling the method so exceptions are rethrown.
		 *
		 * This class is thread safe.
		 */
		class CacheThread: public QThread
		{
			Q_OBJECT

			public:
				CacheThread (Cache &cache);
				virtual ~CacheThread ();

				void refreshAll        (Returner<bool> &returner, OperationMonitor &monitor);
				void fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date);

			signals:
				void sig_refreshAll        (Returner<bool> *returner, OperationMonitor *monitor);
				void sig_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

			protected slots:
				virtual void slot_refreshAll        (Returner<bool> *returner, OperationMonitor *monitor);
				virtual void slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

			protected:
				static const int requestedExit=42;

				virtual void run ();

			private:
				Cache &cache;
		};
	}
}

#endif
