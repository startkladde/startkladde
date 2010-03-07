/*
 * DefaultInterface.h
 *
 *  Created on: 22.02.2010
 *      Author: Martin Herrmann
 */

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QAtomicInt>

#include "src/db/interface/Interface.h"
#include "src/db/DatabaseInfo.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/net/TcpProxy.h"

#ifndef DEFAULTINTERFACE_H_
#define DEFAULTINTERFACE_H_

namespace Db
{
	class Query;

	namespace Interface
	{
		/**
		 * An Interface implementation directly using a QSqlDatabase
		 *
		 * As this class uses a QSqlDatabase, not only is it not thread safe,
		 * it even may only be used in the thread where it was created. Also,
		 * the result returned in the query
		 *
		 * This is
		 * a QtSql restriction, see [1]. For an Interface implementation
		 * without this restriction, see ThreadSafeInterface.
		 *
		 * [1] http://doc.trolltech.com/4.5/threads.html#threads-and-the-sql-module
		 */
		class DefaultInterface: public QObject, public Interface
		{
			Q_OBJECT

			public:
		    	// *** Construction
				DefaultInterface (const DatabaseInfo &dbInfo);
				virtual ~DefaultInterface ();

				// *** Connection management
				virtual bool open ();
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
				QSqlDatabase db;
				TcpProxy *proxy;
				QAtomicInt canceled; // There is no QAtomicBool. 0=false, others=true

				static QAtomicInt freeNumber;
				static int getFreeNumber () { return freeNumber.fetchAndAddOrdered (1); }

				virtual void openImpl ();

				virtual QSqlQuery executeQueryImpl (const Query &query, bool forwardOnly=true);
				virtual QSqlQuery doExecuteQuery (const Query &query, bool forwardOnly=true);

				virtual bool retryOnQueryError (int number);
		};
	}
}

#endif
