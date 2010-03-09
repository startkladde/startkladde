/*
 * CacheWorker.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHEWORKER_H_
#define CACHEWORKER_H_

#include <QObject>
#include <QThread>
#include <QDate>

template<typename T> class Returner;
class OperationMonitor;

class Cache;

/**
 * A background worker to perform cache related work in the background
 *
 * All methods return immediately. The result of the operation is
 * returned using a Returner. The operation can be monitored and
 * canceled (if supported by the operation) through an
 * OperationMonitor. returnedValue or wait must be called on the
 * returner after calling the method so exceptions are rethrown.
 *
 * This class is thread safe.
 *
 * See doc/internal/worker.txt
 */
class CacheWorker: public QObject
{
	Q_OBJECT

	public:
		CacheWorker (Cache &cache);
		virtual ~CacheWorker ();

		void refreshAll        (Returner<void> &returner, OperationMonitor &monitor);
		void fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date);

	signals:
		void sig_refreshAll        (Returner<void> *returner, OperationMonitor *monitor);
		void sig_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

	protected slots:
		virtual void slot_refreshAll        (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date);

	private:
		QThread thread;
		Cache &cache;
};

#endif
