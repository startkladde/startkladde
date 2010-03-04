/*
 * Worker.h
 *
 *  Created on: 03.03.2010
 *      Author: Martin Herrmann
 */

#ifndef MIGRATORWORKER_H_
#define MIGRATORWORKER_H_

#include <QObject>

#include "src/concurrent/Returner.h"

class Migrator;
class OperationMonitor;

namespace Db
{
	namespace Interface { namespace ThreadSafe { class ThreadSafeInterface; } }

	namespace Migration
	{
		namespace Background
		{
			class MigratorWorker: public QObject
			{
				Q_OBJECT

				public:
					// *** Construction
					MigratorWorker (Interface::ThreadSafe::ThreadSafeInterface &interface);
					virtual ~MigratorWorker ();

				public slots:
					virtual void migrate           (Returner<void>            *returner, OperationMonitor *monitor);
					virtual void loadSchema        (Returner<void>            *returner, OperationMonitor *monitor);
					virtual void pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor);
					virtual void isCurrent         (Returner<bool>            *returner, OperationMonitor *monitor);
					virtual void isEmpty           (Returner<bool>            *returner, OperationMonitor *monitor);
					virtual void currentVersion    (Returner<quint64>         *returner, OperationMonitor *monitor);

				private:
					Migrator *migrator;
			};
		}
	}
}

#endif
