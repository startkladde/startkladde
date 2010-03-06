/*
 * Worker.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef WORKER_H_
#define WORKER_H_

#include <QObject>

#include "src/db/interface/AbstractInterface.h"
#include "src/db/Query.h"
#include "src/concurrent/Returner.h"

class QSqlError;
class OperationMonitor;



namespace Db
{
	namespace Interface
	{
		class DefaultInterface;

		namespace ThreadSafe
		{
			/**
			 * Receives signals and performs work using a DefaultInterface
			 *
			 * This class is intended to use in a background thread with queued
			 * signals. Using a DefaultInterface, not only is it not thread
			 * safe, but may on be used in the thread where it was created.
			 *
			 * You probably don't want to use this class directly. The
			 * Interface implementation Db::Interface::ThreadsafeInterface
			 * should be used instead.
			 */
			class Worker: public QObject
			{
				Q_OBJECT;

				public:
					// *** Construction
					Worker (const DatabaseInfo &dbInfo);
					virtual ~Worker ();

				public slots:
					// *** Connection management
					virtual void open      (Returner<bool>      *returner);
					virtual void asyncOpen (Returner<bool>      *returner, OperationMonitor *monitor);
					virtual void close     (Returner<void>      *returner);
					virtual void lastError (Returner<QSqlError> *returner) const;
					virtual void cancelConnection ();

					// *** Transactions
					virtual void transaction (Returner<bool> *returner);
					virtual void commit      (Returner<bool> *returner);
					virtual void rollback    (Returner<bool> *returner);

					// *** Queries
					// Must use Db:: for Query for the signals
					virtual void executeQuery       (Returner<void>                            *returner, Db::Query query);
					virtual void executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, Db::Query query, bool forwardOnly=true);
					virtual void queryHasResult     (Returner<bool>                            *returner, Db::Query query);

				private:
					// FIXME should be Abstract; open with monitor is currently only defined in default
//					AbstractInterface *interface;
					DefaultInterface *interface;
			};
		}
	}
}

#endif
