/*
 * Improvements:
 *   - this could be implemented without subclassing Thread (since Qt 4.4),
 *     like so (see also DbWorker):
 *     class Worker {
 *       QThread thread;
 *       Worker () {
 *         this->moveToThread (&thread);
 *         connect (this, sig_foo, this, slot_foo);
 *         thread->start ();
 *       }
 *       ~Worker () {
 *         thread.quit ();
 *       }
 *       foo () {
 *         emit signal;
 *       }
 *       slot_foo () {
 *         // actual work here
 *       }
 *     }
 */
#include "CacheThread.h"

#include <iostream>

#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/db/cache/Cache.h"
#include "src/concurrent/Returner.h"

namespace Db { namespace Cache {
	CacheThread::CacheThread (Cache &cache):
		cache (cache)
	{
		// First move, then connect
		moveToThread (this);

#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
		CONNECT (refreshAll        (Returner<bool>            *, OperationMonitor *));
		CONNECT (fetchFlightsOther (Returner<void>            *, OperationMonitor *, QDate));
#undef CONNECT

		start ();
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
		int result=exec ();

		if (result!=requestedExit)
			std::cout << "Db::Cache::CacheThread exited" << std::endl;
	}

	// ***********************
	// ** Front-end methods **
	// ***********************

	/**
	 * Calls Cache#refreshAll
	 */
	void CacheThread::refreshAll (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_refreshAll (&returner, &monitor);
	}

	/**
	 * Cache Cache#fetchFlightsOther
	 */
	void CacheThread::fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date)
	{
		emit sig_fetchFlightsOther (&returner, &monitor, date);
	}


	// ********************
	// ** Back-end slots **
	// ********************

	void CacheThread::slot_refreshAll (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, cache.refreshAll (monitor->interface ()));
	}

	void CacheThread::slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date)
	{
		returnVoidOrException (returner, cache.fetchFlightsOther (date, monitor->interface ()));
	}
} }
