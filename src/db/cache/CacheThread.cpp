#include "CacheThread.h"

#include <iostream>

#include "src/concurrent/monitor/OperationMonitor.h"
//#include "src/db/cache/CacheWorker.h"

#include "src/concurrent/Returner.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/db/cache/Cache.h"

namespace Db { namespace Cache {
	CacheThread::CacheThread (Cache &cache):
		worker (NULL), cache (cache)
	{
		moveToThread (this);
		start ();
		waitStartup ();
	}

	CacheThread::~CacheThread ()
	{
		// Terminate the thread's event loop with the requestedExit exit code
		// so it doesn't print a message.
		exit (requestedExit);

		std::cout << "Waiting for cache worker thread to terminate...";
		std::cout.flush ();

		if (wait (1000))
			std::cout << "OK" << std::endl;
		else
			std::cout << "Timeout" << std::endl;
	}

	void CacheThread::run ()
	{
//		worker=new CacheWorker (cache);

#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
		CONNECT (refreshAll        (Returner<bool>            *, OperationMonitor *));
		CONNECT (fetchFlightsOther (Returner<void>            *, OperationMonitor *, QDate));
#undef CONNECT

		startupWaiter.notify ();
		int result=exec ();

		if (result!=requestedExit)
			std::cout << "Db::Cache::CacheThread exited" << std::endl;
	}

	void CacheThread::waitStartup ()
	{
		startupWaiter.wait ();
	}

	// TODO emit signals
	void CacheThread::refreshAll (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_refreshAll (&returner, &monitor);
	}

	void CacheThread::fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date)
	{
		emit sig_fetchFlightsOther (&returner, &monitor, date);
	}

	void CacheThread::slot_refreshAll (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, cache.refreshAll (monitor->interface ()));
	}

	void CacheThread::slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date)
	{
		returnVoidOrException (returner, cache.fetchFlightsOther (date, monitor->interface ()));
	}

} }

