/*
 * Thread.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <QThread>

#include "src/db/interface/threadSafe/Worker.h"

#include "src/concurrent/Waiter.h"
#include "src/db/Query.h"

namespace Db
{
	namespace Interface
	{
		namespace ThreadSafe
		{
			/**
			 *
			 * Note that there is no way to retrieve the worker, as it may only
			 * be used in the thread where it was created.
			 */
			class Thread: public QThread
			{
				Q_OBJECT

				public:
					static const int requestedExit=42;

					Thread (const DatabaseInfo &dbInfo);
					virtual ~Thread ();

					void waitStartup ();

					// These are slots, although you will likely want to call
					// them directly.
					// TODO: have the ThreadSafeInterface emit the signals, just
					// connect them to the worker here
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
					virtual void executeQuery       (Waiter *waiter,                                         const Query &query);
					virtual void executeQueryResult (Waiter *waiter, QSharedPointer<Result::Result> *result, const Query &query, bool forwardOnly=true);
					virtual void queryHasResult     (Waiter *waiter, bool                           *result, const Query &query);

				signals:
					// *** Connection management
					virtual void sig_open (Waiter *waiter, bool *result);
					virtual void sig_close (Waiter *waiter);
					virtual void sig_lastError (Waiter *waiter, QSqlError *result) const;

					// *** Transactions
					virtual void sig_transaction (Waiter *waiter, bool *result);
					virtual void sig_commit      (Waiter *waiter, bool *result);
					virtual void sig_rollback    (Waiter *waiter, bool *result);

					// *** Queries
					// Must use Db:: for Query for the signals
					virtual void sig_executeQuery       (Waiter *waiter,                                         Db::Query query);
					virtual void sig_executeQueryResult (Waiter *waiter, QSharedPointer<Result::Result> *result, Db::Query query, bool forwardOnly=true);
					virtual void sig_queryHasResult     (Waiter *waiter, bool                           *result, Db::Query query);


				protected:
					virtual void run ();

				private:
					Worker *worker;
					DatabaseInfo dbInfo;
					Waiter startupWaiter;
//
//					void connectSignal (const char *signal);
			};
		}
	}
}

#endif
