/*
 * TODO:
 *   - Consider merging BackgroundMigrator and MigratorThread.
 *   - migrate(taking monitor, and returning result) and asynchronousMigrate
 *     (taking monitor and result), so we can directly use a DialogMonitor
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
		// Terminate the thread's event loop with the requestedExit exit code
		// so it doesn't print a message.
		thread.exit (MigratorThread::requestedExit);

		std::cout << "Waiting for migrator worker thread to terminate...";
		std::cout.flush ();

		if (thread.wait (1000))
			std::cout << "OK" << std::endl;
		else
			std::cout << "Timeout" << std::endl;
	}

	void BackgroundMigrator::migrate (Returner<void> &returner, OperationMonitor &monitor)
	{
		thread.migrate (&returner, &monitor);
	}

	void BackgroundMigrator::loadSchema (Returner<void> &returner, OperationMonitor &monitor)
	{
		thread.loadSchema (&returner, &monitor);
	}

	void BackgroundMigrator::pendingMigrations (Returner<QList<quint64> > &returner, OperationMonitor &monitor)
	{
		thread.pendingMigrations (&returner, &monitor);
	}

	void BackgroundMigrator::isCurrent (Returner<bool> &returner, OperationMonitor &monitor)
	{
		thread.isCurrent (&returner, &monitor);
	}

	void BackgroundMigrator::isEmpty (Returner<bool> &returner, OperationMonitor &monitor)
	{
		thread.isEmpty (&returner, &monitor);
	}

	void BackgroundMigrator::currentVersion (Returner<quint64> &returner, OperationMonitor &monitor)
	{
		thread.currentVersion (&returner, &monitor);
	}
} } }
