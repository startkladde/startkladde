/*
 * ThreadSafeInterface.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef THREADSAFEINTERFACE_H_
#define THREADSAFEINTERFACE_H_

#include "src/db/interface/Interface.h"
#include "src/db/interface/threadSafe/Thread.h"

class QSqlError;

namespace Db
{
	namespace Interface
	{
		namespace ThreadSafe
		{
			class ThreadSafeInterface: public Interface
			{
				public:
					// *** Construction
					ThreadSafeInterface (const DatabaseInfo &info);
					virtual ~ThreadSafeInterface ();

					// *** Connection management
					virtual bool open ();
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
					Thread thread;
			};
		}

		typedef ThreadSafe::ThreadSafeInterface ThreadSafeInterface;
	}
}

#endif
