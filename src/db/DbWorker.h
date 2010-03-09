/*
 * DbWorker.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef DBWORKER_H_
#define DBWORKER_H_

#include <QObject>
#include <QThread>

#include "src/db/dbId.h"
#include "src/concurrent/Returner.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"

// FIXME: test deleting of tasks
class Database;

/**
 * This class is thread safe.
 *
 * See doc/internal/worker.txt
 */
class DbWorker: QObject
{
	Q_OBJECT

	public:
		/** Implementation detail, please disregard */
		class Task { public:
			virtual void run (Database &db, OperationMonitor *monitor)=0; };

		DbWorker (Database &db);
		virtual ~DbWorker ();

		template<class T> void createObject (Returner<dbId> &returner, OperationMonitor &monitor, T &object);
		template<class T> void deleteObject (Returner<int > &returner, OperationMonitor &monitor, dbId id);
		template<class T> void updateObject (Returner<int > &returner, OperationMonitor &monitor, const T &object);
		template<class T> void objectUsed   (Returner<bool> &returner, OperationMonitor &monitor, dbId id);

	protected:
		virtual void executeAndDeleteTask (OperationMonitor *monitor, Task *task);

	signals:
		virtual void sig_executeAndDeleteTask (OperationMonitor *monitor, Task *task);

	protected slots:
		virtual void slot_executeAndDeleteTask (OperationMonitor *monitor, Task *task);


	private:
		QThread thread;
		Database &db;
};

#endif
