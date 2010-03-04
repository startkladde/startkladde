#include "MigratorWorker.h"

#include <iostream>

#include "src/db/migration/Migrator.h"
#include "src/db/interface/threadSafe/ThreadSafeInterface.h"

namespace Db
{
	namespace Migration
	{
		namespace Background
		{
			// ******************
			// ** Construction **
			// ******************

			MigratorWorker::MigratorWorker (Interface::ThreadSafe::ThreadSafeInterface &interface):
				migrator (new Migrator (interface))
			{
			}

			MigratorWorker::~MigratorWorker ()
			{
				delete migrator;
			}


			// *************
			// ** Methods **
			// *************

			void MigratorWorker::migrate (Returner<void> *returner, OperationMonitor *monitor)
			{
				returnVoidOrException (returner, migrator->migrate (monitor->interface ()));
			}

			void MigratorWorker::loadSchema (Returner<void> *returner, OperationMonitor *monitor)
			{
				returnVoidOrException (returner, migrator->loadSchema (monitor->interface ()));
			}

			void MigratorWorker::pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor)
			{
				returnOrException (returner, migrator->pendingMigrations (monitor->interface ()));
			}

			void MigratorWorker::isCurrent (Returner<bool> *returner, OperationMonitor *monitor)
			{
				returnOrException (returner, migrator->isCurrent (monitor->interface ()));
			}

			void MigratorWorker::isEmpty (Returner<bool> *returner, OperationMonitor *monitor)
			{
				returnOrException (returner, migrator->isEmpty (monitor->interface ()));
			}

			void MigratorWorker::currentVersion (Returner<quint64> *returner, OperationMonitor *monitor)
			{
				returnOrException (returner, migrator->currentVersion (monitor->interface ()));
			}
		}
	}
}
