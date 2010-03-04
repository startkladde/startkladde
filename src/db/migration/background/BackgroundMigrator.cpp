/*
 * Consider merging BackgroundMigrator and MigratorThread.
 *
 */
#include "BackgroundMigrator.h"

#include <iostream>

#include "src/db/migration/background/MigratorThread.h"
#include "src/concurrent/monitor/OperationMonitor.h"

namespace Db { namespace Migration { namespace Background
{
	BackgroundMigrator::BackgroundMigrator (Interface::ThreadSafe::ThreadSafeInterface &interface):
		thread (interface)
	{
		thread.start ();
		thread.waitStartup ();
	}

	BackgroundMigrator::~BackgroundMigrator ()
	{
		// Terminete the thread's event loop with the requestedExit exit code
		// so it doesn't print a message.
		thread.exit (MigratorThread::requestedExit);

		std::cout << "Waiting for worker thread to terminate...";
		std::cout.flush ();

		if (thread.wait (1000))
			std::cout << "OK" << std::endl;
		else
			std::cout << "Timeout" << std::endl;
	}

	void BackgroundMigrator::migrate (Returner<void> &returner, OperationMonitor &monitor)
	{
//		Returner<void> returner;
//		thread.migrate (&returner, &monitor);
//		returner.wait ();
		thread.migrate (&returner, &monitor);
	}

	void BackgroundMigrator::loadSchema (OperationMonitor &monitor)
	{
		Returner<void> returner;
		thread.loadSchema (&returner, &monitor);
		returner.wait ();
	}

	QList<quint64> BackgroundMigrator::pendingMigrations (OperationMonitor &monitor)
	{
		Returner<QList<quint64> > returner;
		thread.pendingMigrations (&returner, &monitor);
		return returner.returnedValue ();
	}

	bool BackgroundMigrator::isCurrent (OperationMonitor &monitor)
	{
		Returner<bool> returner;
		thread.isCurrent (&returner, &monitor);
		return returner.returnedValue ();
	}

	quint64 BackgroundMigrator::currentVersion (OperationMonitor &monitor)
	{
		Returner<quint64> returner;
		thread.currentVersion (&returner, &monitor);
		return returner.returnedValue ();
	}
} } }
