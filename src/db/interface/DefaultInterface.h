/*
 * DefaultInterface.h
 *
 *  Created on: 22.02.2010
 *      Author: Martin Herrmann
 */

#include <QString>
#include <QStringList>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QAtomicInt>

#include "src/db/interface/Interface.h"
#include "src/db/DatabaseInfo.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"

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
		class DefaultInterface: public Interface
		{
			public:
		    	// *** Construction
				DefaultInterface (const DatabaseInfo &dbInfo);
				virtual ~DefaultInterface ();

				// *** Connection management
				virtual bool open () { return open (OperationMonitorInterface::null); }
				virtual bool open (OperationMonitorInterface monitor);
				virtual void close ();
				virtual QSqlError lastError () const;

				// *** Transactions
				virtual bool transaction ();
				virtual bool commit ();
				virtual bool rollback ();

				// *** Queries
				virtual void executeQuery (const Query &query);
				virtual QSharedPointer<Result::Result> executeQueryResult (const Query &query, bool forwardOnly=true);
				virtual bool queryHasResult (const Query &query);

			private:
				QSqlDatabase db;

				static QAtomicInt freeNumber;
				static int getFreeNumber () { return freeNumber.fetchAndAddOrdered (1); }

				virtual QSqlQuery executeQueryImpl (const Query &query, bool forwardOnly=true);
				virtual QSqlQuery doExecuteQuery (const Query &query, bool forwardOnly=true);
		};
	}
}

#endif
