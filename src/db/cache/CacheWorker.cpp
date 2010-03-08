#include "CacheWorker.h"

#include <iostream>

#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/db/cache/Cache.h"
#include "src/concurrent/Returner.h"

namespace Db { namespace Cache {
	CacheWorker::CacheWorker (Cache &cache):
		cache (cache)
	{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
		CONNECT (refreshAll        (Returner<bool>            *, OperationMonitor *));
		CONNECT (fetchFlightsOther (Returner<void>            *, OperationMonitor *, QDate));
#undef CONNECT

		moveToThread (&thread);
		thread.start ();
	}

	CacheWorker::~CacheWorker ()
	{
		thread.quit ();

		std::cout << "Waiting for cache worker thread to terminate..." << std::flush;
		if (thread.wait (1000)) std::cout << "OK"      << std::endl;
		else                    std::cout << "Timeout" << std::endl;
	}


	// ***********************
	// ** Front-end methods **
	// ***********************

	/**
	 * Calls Cache#refreshAll
	 */
	void CacheWorker::refreshAll (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_refreshAll (&returner, &monitor);
	}

	/**
	 * Cache Cache#fetchFlightsOther
	 */
	void CacheWorker::fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date)
	{
		emit sig_fetchFlightsOther (&returner, &monitor, date);
	}


	// ********************
	// ** Back-end slots **
	// ********************

	void CacheWorker::slot_refreshAll (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, cache.refreshAll (monitor->interface ()));
	}

	void CacheWorker::slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date)
	{
		returnVoidOrException (returner, cache.fetchFlightsOther (date, monitor->interface ()));
	}
} }
