/*
 * FIXME:
 *   - Throw correct exception on access denied etc.
 *   - Allow canceling
 *
 * TODO:
 *   - Handle all relevant errors from
 *     http://dev.mysql.com/doc/refman/5.1/en/error-messages-client.html
 *     http://dev.mysql.com/doc/refman/5.1/en/error-messages-server.html
 *   - Solution for stalled connection
 */
#include "DefaultInterface.h"

#include <iostream>

#include <QVariant>

#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>

#include "src/util/qString.h"
#include "src/db/result/DefaultResult.h"
#include "src/db/Query.h"
#include "src/config/Options.h"
#include "src/text.h"
#include "src/db/interface/exceptions/QueryFailedException.h"
#include "src/db/interface/exceptions/ConnectionFailedException.h"
#include "src/db/interface/exceptions/DatabaseDoesNotExistException.h"
#include "src/db/interface/exceptions/AccessDeniedException.h"

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

	/**
	 * Opens the connection to the database
	 *
	 * This is not required as it is done automatically when a query is
	 * executed and the connection is not open; however, it can be used to
	 * ensure that the database can be reached.
	 *
	 * @return true on success, false else
	 */
	bool DefaultInterface::open (OperationMonitorInterface monitor)
	{
		monitor.status ("Verbindung wird hergestellt");

		DatabaseInfo info=getInfo ();

		db=QSqlDatabase::addDatabase ("QMYSQL");

		db.setHostName     (info.server  );
		db.setUserName     (info.username);
		db.setPassword     (info.password);
		db.setPort         (info.port    );
		db.setDatabaseName (info.database);

		while (true)
		{
			std::cout << QString ("Connecting to %1...").arg (info.toString ());
			std::cout.flush ();

			if (db.open ())
			{
				std::cout << "OK" << std::endl;
				return true;
			}
			else
			{
				QSqlError error=db.lastError ();

				std::cout << error.databaseText () << std::endl;

				switch (error.number ())
				{
					case CR_CONN_HOST_ERROR: break;
					case CR_UNKNOWN_HOST: break;
					case CR_SERVER_LOST: break;
					case ER_BAD_DB_ERROR: throw DatabaseDoesNotExistException (error);
					case ER_ACCESS_DENIED_ERROR: throw AccessDeniedException (error);
					case ER_DBACCESS_DENIED_ERROR: throw AccessDeniedException (error);
					default: throw ConnectionFailedException (error);
				}

				monitor.status (QString ("Verbindung wird hergestellt\nFehler: %1").arg (error.databaseText ()));

				sleep (1);
				std::cout << "Retrying...";
			}
		}
	}

	// TODO should be RAII, but calling this in the destructor doesn't seem to
	// work (specifically, destroying db)
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

	QSqlQuery DefaultInterface::executeQueryImpl (const Query &query, bool forwardOnly)
	{
		while (true)
		{
			try
			{
				if (!db.isOpen ()) open ();

				return doExecuteQuery (query, forwardOnly);
			}
			catch (QueryFailedException &ex)
			{
				switch (ex.error.number ())
				{
					case CR_SERVER_GONE_ERROR: break;
					case CR_SERVER_LOST: break;
					default: throw;
				}

				close ();

				if (opts.display_queries) std::cout << "Retrying...";
			}
		}
	}


	/**
	 * Executes a query and returns the QSqlQuery
	 *
	 * @throw QueryFailedException if the query fails
	 */
	QSqlQuery DefaultInterface::doExecuteQuery (const Query &query, bool forwardOnly)
	{
		if (opts.display_queries)
		{
			std::cout << query.colorizedString ();
			std::cout.flush ();
		}

		QSqlQuery sqlQuery (db);
		sqlQuery.setForwardOnly (forwardOnly);

		if (!query.prepare (sqlQuery))
		{
			QSqlError error=sqlQuery.lastError ();

			if (opts.display_queries)
				std::cout << error.databaseText () << std::endl;

			throw QueryFailedException::prepare (error, query);
		}

		query.bindTo (sqlQuery);

		if (opts.display_queries)
			std::cout << "..."; std::cout.flush ();

		if (!sqlQuery.exec ())
		{
			QSqlError error=sqlQuery.lastError ();

			if (opts.display_queries)
				std::cout << error.databaseText () << std::endl;

			throw QueryFailedException::execute (error, query);
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
