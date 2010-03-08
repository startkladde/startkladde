#include "BackgroundMigrator.h"

#include <iostream>

#include "src/db/migration/Migrator.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/db/interface/threadSafe/ThreadSafeInterface.h" // required for Interface inheritance

namespace Db { namespace Migration { namespace Background
{
	BackgroundMigrator::BackgroundMigrator (Interface::ThreadSafe::ThreadSafeInterface &interface):
		migrator (interface)
	{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
		CONNECT (migrate           (Returner<void>            *, OperationMonitor *));
		CONNECT (loadSchema        (Returner<void>            *, OperationMonitor *));
		CONNECT (pendingMigrations (Returner<QList<quint64> > *, OperationMonitor *));
		CONNECT (isCurrent         (Returner<bool>            *, OperationMonitor *));
		CONNECT (isEmpty           (Returner<bool>            *, OperationMonitor *));
		CONNECT (currentVersion    (Returner<quint64>         *, OperationMonitor *));
#undef CONNECT

		moveToThread (&thread);
		thread.start ();
	}

	BackgroundMigrator::~BackgroundMigrator ()
	{
		thread.quit ();

		std::cout << "Waiting for migrator worker thread to terminate..." << std::flush;
		if (thread.wait (1000)) std::cout << "OK"      << std::endl;
		else                    std::cout << "Timeout" << std::endl;
	}

	// ***********************
	// ** Front-end methods **
	// ***********************

	void BackgroundMigrator::migrate (Returner<void> &returner, OperationMonitor &monitor)
	{
		emit sig_migrate (&returner, &monitor);
	}

	void BackgroundMigrator::loadSchema (Returner<void> &returner, OperationMonitor &monitor)
	{
		emit sig_loadSchema (&returner, &monitor);
	}

	void BackgroundMigrator::pendingMigrations (Returner<QList<quint64> > &returner, OperationMonitor &monitor)
	{
		emit sig_pendingMigrations (&returner, &monitor);
	}

	void BackgroundMigrator::isCurrent (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_isCurrent (&returner, &monitor);
	}

	void BackgroundMigrator::isEmpty (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_isEmpty (&returner, &monitor);
	}

	void BackgroundMigrator::currentVersion (Returner<quint64> &returner, OperationMonitor &monitor)
	{
		emit sig_currentVersion (&returner, &monitor);
	}


	// ********************
	// ** Back-end slots **
	// ********************

	void BackgroundMigrator::slot_migrate (Returner<void> *returner, OperationMonitor *monitor)
	{
		returnVoidOrException (returner, migrator.migrate (monitor->interface ()));
	}

	void BackgroundMigrator::slot_loadSchema (Returner<void> *returner, OperationMonitor *monitor)
	{
		returnVoidOrException (returner, migrator.loadSchema (monitor->interface ()));
	}

	void BackgroundMigrator::slot_pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.pendingMigrations (monitor->interface ()));
	}

	void BackgroundMigrator::slot_isCurrent (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.isCurrent (monitor->interface ()));
	}

	void BackgroundMigrator::slot_isEmpty (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.isEmpty (monitor->interface ()));
	}

	void BackgroundMigrator::slot_currentVersion (Returner<quint64> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.currentVersion (monitor->interface ()));
	}

} } }
