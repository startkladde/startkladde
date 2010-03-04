#include "MigratorThread.h"

#include <iostream>

#include "src/db/migration/background/MigratorThread.h"
#include "src/db/migration/background/MigratorWorker.h"

namespace Db { namespace Migration { namespace Background
{
	MigratorThread::MigratorThread (Interface::ThreadSafe::ThreadSafeInterface &interface):
		worker (NULL), interface (interface)
	{
	}

	MigratorThread::~MigratorThread ()
	{
	}

	void MigratorThread::run ()
	{
		worker=new MigratorWorker (interface);

#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), worker, SLOT (definition))
		CONNECT (migrate           (Returner<void>            *, OperationMonitor *));
		CONNECT (loadSchema        (Returner<void>            *, OperationMonitor *));
		CONNECT (pendingMigrations (Returner<QList<quint64> > *, OperationMonitor *));
		CONNECT (isCurrent         (Returner<bool>            *, OperationMonitor *));
		CONNECT (isEmpty           (Returner<bool>            *, OperationMonitor *));
		CONNECT (currentVersion    (Returner<quint64>         *, OperationMonitor *));
#undef CONNECT

		startupWaiter.notify ();
		int result=exec ();

		if (result!=requestedExit)
			std::cout << "Db::Interface::ThreadSafe::Thread exited" << std::endl;
	}

	void MigratorThread::waitStartup ()
	{
		startupWaiter.wait ();
	}


	// ***************************
	// ** Connection management **
	// ***************************

	void MigratorThread::migrate (Returner<void> *returner, OperationMonitor *monitor)
	{
		emit sig_migrate (returner, monitor);
	}

	void MigratorThread::loadSchema (Returner<void> *returner, OperationMonitor *monitor)
	{
		emit sig_loadSchema (returner, monitor);
	}

	void MigratorThread::pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor)
	{
		emit sig_pendingMigrations (returner, monitor);
	}

	void MigratorThread::isCurrent (Returner<bool> *returner, OperationMonitor *monitor)
	{
		emit sig_isCurrent (returner, monitor);
	}

	void MigratorThread::isEmpty (Returner<bool> *returner, OperationMonitor *monitor)
	{
		emit sig_isEmpty (returner, monitor);
	}

	void MigratorThread::currentVersion (Returner<quint64> *returner, OperationMonitor *monitor)
	{
		emit sig_currentVersion (returner, monitor);
	}
} } }
