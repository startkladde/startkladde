/*
 * MigratorWorker.h
 *
 *  Created on: 03.03.2010
 *      Author: Martin Herrmann
 */

#ifndef MIGRATORWORKER_H
#define MIGRATORWORKER_H

#include <QThread>

#include "src/concurrent/Returner.h"
#include "src/db/migration/Migrator.h"

class OperationMonitor;

namespace Db
{
	namespace Interface { namespace ThreadSafe { class ThreadSafeInterface; } }

	namespace Migration
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
		class MigratorWorker: public QObject
		{
			Q_OBJECT

			public:
				MigratorWorker (Interface::ThreadSafe::ThreadSafeInterface &interface);
				virtual ~MigratorWorker ();

				void migrate           (Returner<void>            &returner, OperationMonitor &monitor);
				void loadSchema        (Returner<void>            &returner, OperationMonitor &monitor);
				void pendingMigrations (Returner<QList<quint64> > &returner, OperationMonitor &monitor);
				void isCurrent         (Returner<bool>            &returner, OperationMonitor &monitor);
				void isEmpty           (Returner<bool>            &returner, OperationMonitor &monitor);
				void currentVersion    (Returner<quint64>         &returner, OperationMonitor &monitor);

			signals:
				virtual void sig_migrate           (Returner<void>            *returner, OperationMonitor *monitor);
				virtual void sig_loadSchema        (Returner<void>            *returner, OperationMonitor *monitor);
				virtual void sig_pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor);
				virtual void sig_isCurrent         (Returner<bool>            *returner, OperationMonitor *monitor);
				virtual void sig_isEmpty           (Returner<bool>            *returner, OperationMonitor *monitor);
				virtual void sig_currentVersion    (Returner<quint64>         *returner, OperationMonitor *monitor);

			protected slots:
				virtual void slot_migrate           (Returner<void>            *returner, OperationMonitor *monitor);
				virtual void slot_loadSchema        (Returner<void>            *returner, OperationMonitor *monitor);
				virtual void slot_pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor);
				virtual void slot_isCurrent         (Returner<bool>            *returner, OperationMonitor *monitor);
				virtual void slot_isEmpty           (Returner<bool>            *returner, OperationMonitor *monitor);
				virtual void slot_currentVersion    (Returner<quint64>         *returner, OperationMonitor *monitor);


			private:
				QThread thread;
				Migrator migrator;
		};
	}
}

#endif
