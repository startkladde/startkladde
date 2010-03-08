#include "MigratorWorker.h"

#include <iostream>

#include "src/db/migration/Migrator.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/db/interface/ThreadSafeInterface.h" // required for Interface inheritance

namespace Db { namespace Migration
{
	MigratorWorker::MigratorWorker (Interface::ThreadSafeInterface &interface):
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

	MigratorWorker::~MigratorWorker ()
	{
		thread.quit ();

		std::cout << "Waiting for migrator worker thread to terminate..." << std::flush;
		if (thread.wait (1000)) std::cout << "OK"      << std::endl;
		else                    std::cout << "Timeout" << std::endl;
	}

	// ***********************
	// ** Front-end methods **
	// ***********************

	void MigratorWorker::migrate (Returner<void> &returner, OperationMonitor &monitor)
	{
		emit sig_migrate (&returner, &monitor);
	}

	void MigratorWorker::loadSchema (Returner<void> &returner, OperationMonitor &monitor)
	{
		emit sig_loadSchema (&returner, &monitor);
	}

	void MigratorWorker::pendingMigrations (Returner<QList<quint64> > &returner, OperationMonitor &monitor)
	{
		emit sig_pendingMigrations (&returner, &monitor);
	}

	void MigratorWorker::isCurrent (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_isCurrent (&returner, &monitor);
	}

	void MigratorWorker::isEmpty (Returner<bool> &returner, OperationMonitor &monitor)
	{
		emit sig_isEmpty (&returner, &monitor);
	}

	void MigratorWorker::currentVersion (Returner<quint64> &returner, OperationMonitor &monitor)
	{
		emit sig_currentVersion (&returner, &monitor);
	}


	// ********************
	// ** Back-end slots **
	// ********************

	void MigratorWorker::slot_migrate (Returner<void> *returner, OperationMonitor *monitor)
	{
		returnVoidOrException (returner, migrator.migrate (monitor->interface ()));
	}

	void MigratorWorker::slot_loadSchema (Returner<void> *returner, OperationMonitor *monitor)
	{
		returnVoidOrException (returner, migrator.loadSchema (monitor->interface ()));
	}

	void MigratorWorker::slot_pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.pendingMigrations (monitor->interface ()));
	}

	void MigratorWorker::slot_isCurrent (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.isCurrent (monitor->interface ()));
	}

	void MigratorWorker::slot_isEmpty (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.isEmpty (monitor->interface ()));
	}

	void MigratorWorker::slot_currentVersion (Returner<quint64> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, migrator.currentVersion (monitor->interface ()));
	}

} }
