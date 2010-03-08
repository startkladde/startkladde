/*
 * ThreadSafeInterface.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef THREADSAFEINTERFACE_H_
#define THREADSAFEINTERFACE_H_

#include <QObject>
#include <QThread>

#include "src/db/interface/Interface.h"
#include "src/db/Query.h" // required for passing a query by copy in a signal

class QSqlError;

template<typename T> class Returner;
class OperationMonitor;

namespace Db
{
	namespace Interface
	{
		class ThreadSafeInterface: public QObject, public Interface
		{
			Q_OBJECT

			public:
				// *** Construction
				ThreadSafeInterface (const DatabaseInfo &info);
				virtual ~ThreadSafeInterface ();

			public:
				// Connection management
				virtual bool open ();
				virtual void asyncOpen (Returner<bool> &returner, OperationMonitor &monitor);
				virtual void close ();
				virtual QSqlError lastError () const;
				// Transactions
				virtual void transaction ();
				virtual void commit ();
				virtual void rollback ();
				// Queries
				virtual void executeQuery (const Query &query);
				virtual QSharedPointer<Result::Result> executeQueryResult (const Query &query, bool forwardOnly=true);
				virtual bool queryHasResult (const Query &query);

			public slots:
				virtual void cancelConnection ();

			signals:
				// Connection management
				virtual void sig_open      (Returner<bool>      *returner);
				virtual void sig_asyncOpen (Returner<bool>      *returner, OperationMonitor *monitor);
				virtual void sig_close     (Returner<void>      *returner);
				virtual void sig_lastError (Returner<QSqlError> *returner) const;
				// Transactions
				virtual void sig_transaction (Returner<void> *returner);
				virtual void sig_commit      (Returner<void> *returner);
				virtual void sig_rollback    (Returner<void> *returner);
				// Queries
				virtual void sig_executeQuery       (Returner<void>                            *returner, Db::Query query); // full type name
				virtual void sig_executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, Db::Query query, bool forwardOnly=true); // full type name
				virtual void sig_queryHasResult     (Returner<bool>                            *returner, Db::Query query); // full type name

				void databaseError (int number, QString message);

			protected slots:
				// *** Connection management
				virtual void slot_open      (Returner<bool>      *returner);
				virtual void slot_asyncOpen (Returner<bool>      *returner, OperationMonitor *monitor);
				virtual void slot_close     (Returner<void>      *returner);
				virtual void slot_lastError (Returner<QSqlError> *returner) const;
				// *** Transactions
				virtual void slot_transaction (Returner<void> *returner);
				virtual void slot_commit      (Returner<void> *returner);
				virtual void slot_rollback    (Returner<void> *returner);
				// *** Queries
				virtual void slot_executeQuery       (Returner<void>                            *returner, Db::Query query); // full type name
				virtual void slot_executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, Db::Query query, bool forwardOnly=true); // full type name
				virtual void slot_queryHasResult     (Returner<bool>                            *returner, Db::Query query); // full type name

			private:
				QThread thread;
				AbstractInterface *interface;
		};
	}
}

#endif
