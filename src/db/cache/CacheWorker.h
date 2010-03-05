/*
 * CacheWorker.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHEWORKER_H_
#define CACHEWORKER_H_

#include <QObject>
#include <QDate>

#include "src/concurrent/Returner.h"

class OperationMonitor;

namespace Db
{
	namespace Cache
	{
		class Cache;

		class CacheWorker: public QObject
		{
			Q_OBJECT

			public:
				CacheWorker (Cache &cache);
				virtual ~CacheWorker ();

			public slots:
				virtual void slot_refreshAll       (Returner<bool> *returner, OperationMonitor *monitor);
				virtual void slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

			private:
				Cache &cache;
		};
	}
}

#endif
