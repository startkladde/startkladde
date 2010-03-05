/*
 * CacheThread.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHETHREAD_H_
#define CACHETHREAD_H_

#include <QObject>
#include <QThread>
#include <QDate>

#include "src/concurrent/Waiter.h"

template<typename T> class Returner;
class OperationMonitor;

namespace Db
{
	namespace Cache
	{
		class Cache;
		class CacheWorker;

		class CacheThread: public QThread
		{
			Q_OBJECT

			public:
				static const int requestedExit=42;

				CacheThread (Cache &cache);
				virtual ~CacheThread ();
				void waitStartup ();

				void refreshAll (Returner<bool> &returner, OperationMonitor &monitor);
				void fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date);

			signals:
				void sig_refreshAll (Returner<bool> *returner, OperationMonitor *monitor);
				void sig_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

			protected slots:
				virtual void slot_refreshAll       (Returner<bool> *returner, OperationMonitor *monitor);
				virtual void slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

			protected:
				virtual void run ();

			private:
				CacheWorker *worker; // Must be created in run() // TODO
				Cache &cache;
				Waiter startupWaiter;
		};
	}
}

#endif
