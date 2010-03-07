/*
 * ThreadSafeInterface.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef THREADSAFEINTERFACE_H_
#define THREADSAFEINTERFACE_H_

#include <QObject>

#include "src/db/interface/Interface.h"
#include "src/db/interface/threadSafe/Thread.h"

class QSqlError;

template<typename T> class Returner;
class OperationMonitor;

namespace Db
{
	namespace Interface
	{
		namespace ThreadSafe
		{
			class ThreadSafeInterface: public QObject, public Interface
			{
				Q_OBJECT

				public:
					// *** Construction
					ThreadSafeInterface (const DatabaseInfo &info);
					virtual ~ThreadSafeInterface ();

				public slots:
					// *** Connection management
					virtual bool open ();
					virtual void asyncOpen (Returner<bool> &returner, OperationMonitor &monitor);
					virtual void close ();
					virtual QSqlError lastError () const;
					virtual void cancelConnection ();

					// *** Transactions
					virtual void transaction ();
					virtual void commit ();
					virtual void rollback ();

					// *** Queries
					virtual void executeQuery (const Query &query);
					virtual QSharedPointer<Result::Result> executeQueryResult (const Query &query, bool forwardOnly=true);
					virtual bool queryHasResult (const Query &query);

				signals:
					void databaseError (int number, QString message);

				private:
					Thread thread;
			};
		}

		typedef ThreadSafe::ThreadSafeInterface ThreadSafeInterface;
	}
}

#endif
