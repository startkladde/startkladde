#include "DefaultInterface.h"

#include <iostream>

#include <QVariant>

#include "src/util/qString.h"
#include "src/db/result/DefaultResult.h"
#include "src/db/Query.h"
#include "src/config/Options.h"
#include "src/text.h"

namespace Db { namespace Interface
{
	// ******************
	// ** Construction **
	// ******************

	DefaultInterface::DefaultInterface (const DatabaseInfo &dbInfo):
		Interface (dbInfo)
	{
	}

	DefaultInterface::~DefaultInterface ()
	{
	}


	// ***************************
	// ** Connection management **
	// ***************************

	bool DefaultInterface::open ()
	{
		db=QSqlDatabase::addDatabase ("QMYSQL");

		const DatabaseInfo &info=getInfo ();

		std::cout << QString ("Connecting to %1@%2:%3")
			.arg (info.username, info.server, info.database) << std::endl;

		db.setHostName     (info.server  );
		db.setUserName     (info.username);
		db.setPassword     (info.password);
		db.setPort         (info.port    );
		db.setDatabaseName (info.database);

		return db.open ();
	}

	void DefaultInterface::close ()
	{
		std::cout << "Closing connection" << std::endl;

		db.close ();

		// Make sure the QSqlDatabase instance is destroyed before removing it
		db=QSqlDatabase ();
		QSqlDatabase::removeDatabase (db.connectionName ());
	}

	QSqlError DefaultInterface::lastError () const
	{
		return db.lastError ();
	}


	// ******************
	// ** Transactions **
	// ******************

	bool DefaultInterface::transaction ()
	{
		return db.transaction ();
	}

	bool DefaultInterface::commit ()
	{
		return db.commit ();
	}

	bool DefaultInterface::rollback ()
	{
		return db.rollback ();
	}


	// *************
	// ** Queries **
	// *************

	/**
	 * Executes a query
	 *
	 * @param query the query to execute
	 * @param forwardOnly the forwardOnly flag to set on the query
	 * @return a pointer to the result of query; owned by query
	 * @throw QueryFailedException if the query fails
	 */
	void DefaultInterface::executeQuery (const Query &query)
	{
		executeQueryImpl (query);
	}

	/**
	 * Executes a query and returns the result
	 *
	 * @param query the query to execute
	 * @param forwardOnly the forwardOnly flag to set on the query
	 * @return TODO
	 * @throw QueryFailedException if the query fails
	 */
	QSharedPointer<Result::Result> DefaultInterface::executeQueryResult (const Query &query, bool forwardOnly)
	{
		QSqlQuery sqlQuery=executeQueryImpl (query, forwardOnly);

		return QSharedPointer<Result::Result> (
			new Result::DefaultResult (sqlQuery));
	}

	/**
	 * Executes a query and returns whether the query had a result (i. e. the
	 * result set is not empty)
	 *
	 * @param query
	 * @return
	 */
	bool DefaultInterface::queryHasResult (const Query &query)
	{
		return executeQueryImpl (query, true).size ()>0;
	}

	/**
	 * Executes a query and returns the QSqlQuery
	 */
	QSqlQuery DefaultInterface::executeQueryImpl (const Query &query, bool forwardOnly)
	{
		if (opts.display_queries)
		{
			std::cout << query.colorizedString () << "...";
			std::cout.flush ();
		}

		QSqlQuery sqlQuery (db);
		sqlQuery.setForwardOnly (forwardOnly);
		query.prepare (sqlQuery);
		query.bindTo (sqlQuery);

		if (!sqlQuery.exec ())
		{
			if (opts.display_queries)
				std::cout << "Query failed" << std::endl;

			throw QueryFailedException (sqlQuery);
		}
		else
		{
			if (opts.display_queries)
			{
				if (sqlQuery.isSelect ())
					std::cout << countText (sqlQuery.size (), "row", "rows") << " returned" << std::endl;
				else
					std::cout << countText (sqlQuery.numRowsAffected (), "row", "rows") << " affected" << std::endl;

			}

			return sqlQuery;
		}

	}
} }
