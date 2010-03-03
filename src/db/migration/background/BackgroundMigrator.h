/*
 * BackgroundMigrator.h
 *
 *  Created on: 03.03.2010
 *      Author: Martin Herrmann
 */

#ifndef BACKGROUNDMIGRATOR_H_
#define BACKGROUNDMIGRATOR_H_

#include "src/db/interface/threadSafe/ThreadSafeInterface.h"
#include "src/db/migration/background/MigratorThread.h"
#include "src/concurrent/Returner.h"

class OperationMonitor;

namespace Db
{
	namespace Migration
	{
		namespace Background
		{
			/**
			 * A class for running migrations in the background, using a Migrator
			 * in a background thread.
			 *
			 * Calls to methods of this class are not blocking.
			 *
			 * Note that this class does not share a common base class with
			 * Migrator and only implements a subsets of its public methods.
			 * This may be changed in the future. The method it implements have
			 * the same interface as those of Migrator, though.
			 *
			 * The background thread implementation is similar to
			 * ThreadSafeInterface; there are more comments about the
			 * implementation there.
			 */
			class BackgroundMigrator
			{
				public:
					BackgroundMigrator (Interface::ThreadSafe::ThreadSafeInterface &interface);
					virtual ~BackgroundMigrator ();

					void migrate                     (Returner<void> &returner, OperationMonitor &monitor);
					void loadSchema                  (OperationMonitor &monitor);
					QList<quint64> pendingMigrations (OperationMonitor &monitor);
					bool isCurrent                   (OperationMonitor &monitor);
					quint64 currentVersion           (OperationMonitor &monitor);

				private:
					Db::Migration::Background::MigratorThread thread;
			};
		}
	}
}

#endif
