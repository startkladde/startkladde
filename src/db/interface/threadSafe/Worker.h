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

class Waiter; // TODO remove

namespace Db
{
	namespace Interface
	{
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
					virtual void open (Waiter *waiter, bool *result);
					virtual void close (Waiter *waiter);
					virtual void lastError (Waiter *waiter, QSqlError *result) const;

					// *** Transactions
					virtual void transaction (Waiter *waiter, bool *result);
					virtual void commit      (Waiter *waiter, bool *result);
					virtual void rollback    (Waiter *waiter, bool *result);

					// *** Queries
					// Must use Db:: for Query for the signals
					virtual void executeQuery       (Waiter *waiter,                                         Db::Query query);
					virtual void executeQueryResult (Waiter *waiter, QSharedPointer<Result::Result> *result, Db::Query query, bool forwardOnly=true);
//					virtual void queryHasResult     (Waiter *waiter, bool                           *result, Db::Query query);
					virtual void queryHasResult     (Returner<bool> *returner        , Db::Query query);

				private:
					AbstractInterface *interface;
			};
		}
	}
}

#endif
