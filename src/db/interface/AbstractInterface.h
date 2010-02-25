/*
 * AbstractInterface.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef ABSTRACTINTERFACE_H_
#define ABSTRACTINTERFACE_H_

#include <QSqlQuery> // TODO remove after fixing QueryFailedException

#include "src/db/DatabaseInfo.h"

namespace Db
{
	class Query;
	namespace Result { class Result; }

	namespace Interface
	{
		// TODO own file
		class QueryFailedException
		{
			public:
				QueryFailedException (QSqlQuery query): query (query) {}
				QSqlQuery query; // FIXME: may not access query from other thread
		};

		/**
		 * A low level interface to the database, capable of managing a
		 * connection and executing queries.
		 *
		 * It also provides abstractions for different database backends (even
		 * though QSqlDatabase is used, there are still some differences
		 * between backends, for example the connection parameters and the data
		 * types).
		 */
		class AbstractInterface
		{
			public:
				AbstractInterface (const DatabaseInfo &info);
				virtual ~AbstractInterface ();

				// *** Connection management
				virtual bool open ()=0;
				virtual void close ()=0;
				virtual QSqlError lastError () const=0;
				virtual const DatabaseInfo &getInfo () const;

				// *** Transactions
				virtual bool transaction ()=0;
				virtual bool commit ()=0;
				virtual bool rollback ()=0;

				// *** Queries
				virtual Result::Result *executeQuery (const Query &query, bool forwardOnly=true)=0;
				virtual bool queryHasResult (const Query &query)=0;

			private:
				DatabaseInfo info;
		};
	}
}

#endif
