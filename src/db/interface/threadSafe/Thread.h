/*
 * TODO:
 *   - consider creating and returning the Returner here, so we can write
 *     return thread.queryHasResult (query).returnedValue () in
 *     ThreadSafeInterface (requires copying Returner)
 */
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
#include "src/concurrent/Returner.h"

class QSqlError;

class OperationMonitor;

namespace Db
{
	namespace Interface
	{
		class QueryFailedException;

		namespace ThreadSafe
		{
			/**
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

					// *** Connection management
					virtual void open      (Returner<bool>      *returner);
					virtual void asyncOpen (Returner<bool>      *returner, OperationMonitor *monitor);
					virtual void close     (Returner<void>      *returner);
					virtual void lastError (Returner<QSqlError> *returner) const;
					virtual void cancelConnection ();

					// *** Transactions
					virtual void transaction (Returner<void> *returner);
					virtual void commit      (Returner<void> *returner);
					virtual void rollback    (Returner<void> *returner);

					// *** Queries
					virtual void executeQuery       (Returner<void>                            *returner, const Query &query);
					virtual void executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, const Query &query, bool forwardOnly=true);
					virtual void queryHasResult     (Returner<bool>                            *returner, const Query &query);

				signals:
					// *** Connection management
					virtual void sig_open      (Returner<bool>      *returner);
					virtual void sig_asyncOpen (Returner<bool>      *returner, OperationMonitor *monitor);
					virtual void sig_close     (Returner<void>      *returner);
					virtual void sig_lastError (Returner<QSqlError> *returner) const;

					// *** Transactions
					virtual void sig_transaction (Returner<void> *returner);
					virtual void sig_commit      (Returner<void> *returner);
					virtual void sig_rollback    (Returner<void> *returner);

					// *** Queries
					virtual void sig_executeQuery       (Returner<void>                            *returner, Db::Query query);
					virtual void sig_executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, Db::Query query, bool forwardOnly=true);
					virtual void sig_queryHasResult     (Returner<bool>                            *returner, Db::Query query);


				protected:
					virtual void run ();

				private:
					Worker *worker;
					DatabaseInfo dbInfo;
					Waiter startupWaiter;
			};
		}
	}
}

#endif
