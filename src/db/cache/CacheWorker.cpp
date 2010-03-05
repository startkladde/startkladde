#include "CacheWorker.h"

#include <iostream>

#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/db/cache/Cache.h"

#include "src/util/qString.h"

namespace Db { namespace Cache
{
	CacheWorker::CacheWorker (Cache &cache):
		cache (cache)
	{
	}

	CacheWorker::~CacheWorker ()
	{
	}

	void CacheWorker::slot_refreshAll (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, cache.refreshAll (monitor->interface ()));
	}

	void CacheWorker::slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date)
	{
		returnVoidOrException (returner, cache.fetchFlightsOther (date, monitor->interface ()));
	}

} }
