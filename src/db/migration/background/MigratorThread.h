/*
 * Thread.h
 *
 *  Created on: 03.03.2010
 *      Author: Martin Herrmann
 */

#ifndef MIGRATORTHREAD_H_
#define MIGRATORTHREAD_H_

#include <QObject>
#include <QThread>

#include "src/concurrent/Waiter.h"
#include "src/concurrent/Returner.h"
#include "src/concurrent/monitor/OperationMonitor.h"

namespace Db
{
	namespace Interface { namespace ThreadSafe { class ThreadSafeInterface; } }

	namespace Migration
	{
		namespace Background
		{
			class MigratorWorker;

			/**
			 * Note that we have to use pointers rather than references to the
			 * returner and the monitor because the signal is queued for
			 * delivery on another thread, so the data types have to be
			 * assignable.
			 */
			class MigratorThread: public QThread
			{
				Q_OBJECT

				public:
					static const int requestedExit=42;

					MigratorThread (Interface::ThreadSafe::ThreadSafeInterface &interface);
					virtual ~MigratorThread ();
					void waitStartup ();

					virtual void migrate           (Returner<void>            *returner, OperationMonitor *monitor);
					virtual void loadSchema        (Returner<void>            *returner, OperationMonitor *monitor);
					virtual void pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor);
					virtual void isCurrent         (Returner<bool>            *returner, OperationMonitor *monitor);
					virtual void isEmpty           (Returner<bool>            *returner, OperationMonitor *monitor);
					virtual void currentVersion    (Returner<quint64>         *returner, OperationMonitor *monitor);

				signals:
					virtual void sig_migrate           (Returner<void>            *returner, OperationMonitor *monitor);
					virtual void sig_loadSchema        (Returner<void>            *returner, OperationMonitor *monitor);
					virtual void sig_pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor);
					virtual void sig_isCurrent         (Returner<bool>            *returner, OperationMonitor *monitor);
					virtual void sig_isEmpty           (Returner<bool>            *returner, OperationMonitor *monitor);
					virtual void sig_currentVersion    (Returner<quint64>         *returner, OperationMonitor *monitor);

				protected:
					virtual void run ();

				private:
					MigratorWorker *worker; // Must be created in run()
					Interface::ThreadSafe::ThreadSafeInterface &interface;
					Waiter startupWaiter;
			};
		}
	}
}

#endif
