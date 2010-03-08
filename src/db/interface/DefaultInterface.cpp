/*
 * TODO:
 *   - Handle all relevant errors from
 *     http://dev.mysql.com/doc/refman/5.1/en/error-messages-client.html
 *     http://dev.mysql.com/doc/refman/5.1/en/error-messages-server.html
 *   - Solution for stalled connection
 */


/*
 * On synchronization:
 *   - The canceled flag is reset at the beginning of an operation. If the
 *     canceled flag was reset right before the operation, cancelConnection
 *     could be called again before the CanceledException has propagated to the
 *     calling thread, thereby setting canceled again and causing the next
 *     operation to cancel prematurely.
 *     This means that a cancelConnection call *before* the start of the
 *     operation will not cause the operation to cancel. This is consistent
 *     with the fact that the connection (through the proxy) may not have been
 *     established at this point and thus may not be canceled.
 *     This is also plausible as the cancelation by the user is unlikely to
 *     occur before the start of the operation, and even if it is, the user may
 *     cancel again after the start of the operation.
 *     Allowing the operation to be canceled before it has started would
 *     certainly be an improvement, but it is probably quite hard to do without
 *     getting a race condition.
 *     Note that there is still a race condition when calling one method from
 *     another (e. g. open from executeQueryImpl on reconnect) and the cancel
 *     flag is set: the cancel will not be performed because there is no
 *     connection. The use will have to cancel again in this case.
 *
 * On Reconnect:
 *   - Test case: start, connect; low limit (30 Bytes/s) in ThrottleProxy;
 *     disconnect in ThrottleProxy; refresh (connection must be reopened);
 *     cancel (must cancel immediately)
 */
#include "DefaultInterface.h"

#include <iostream>

#include <QVariant>

#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>

#include "src/util/qString.h"
#include "src/db/result/DefaultResult.h"
#include "src/config/Options.h"
#include "src/text.h"
#include "src/db/interface/exceptions/QueryFailedException.h"
#include "src/db/interface/exceptions/ConnectionFailedException.h"
#include "src/db/interface/exceptions/DatabaseDoesNotExistException.h"
#include "src/db/interface/exceptions/AccessDeniedException.h"
#include "src/concurrent/monitor/OperationCanceledException.h"

namespace Db { namespace Interface
{
	QAtomicInt DefaultInterface::freeNumber=0;

	// ******************
	// ** Construction **
	// ******************

	DefaultInterface::DefaultInterface (const DatabaseInfo &dbInfo):
		Interface (dbInfo)
	{
		proxy=new TcpProxy ();

		QString name=utf8 ("startkladde_defaultInterface_%1").arg (getFreeNumber ());
		//std::cout << "Create db " << name << std::endl;

		db=QSqlDatabase::addDatabase ("QMYSQL", name);
	}

	DefaultInterface::~DefaultInterface ()
	{
		if (db.isOpen ()) db.close ();

		QString name=db.connectionName ();

		// Make sure the QSqlDatabase instance is destroyed before removing it
		db=QSqlDatabase ();

		//std::cout << "remove db " << name << std::endl;
		QSqlDatabase::removeDatabase (name);

		delete proxy;
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
	// TODO remove monitor
	bool DefaultInterface::open ()
	{
		// Reset the canceled flag
		canceled=0;

		openImpl ();
		return true;
	}

	void DefaultInterface::openImpl ()
	{
		DatabaseInfo info=getInfo ();

		// TODO handle proxy port=0: throw an exception, but we don't have an
		// SqlError, so it's not an SqlException
		quint16 proxyPort=proxy->open (info.server, info.port);

//		db.setHostName     (info.server  );
		db.setHostName     ("127.0.0.1");
		db.setUserName     (info.username);
		db.setPassword     (info.password);
//		db.setPort         (info.port    );
		db.setPort         (proxyPort);
		db.setDatabaseName (info.database);

		while (true)
		{
			std::cout << QString ("%1 connecting to %2 via %3:%4...")
				.arg (db.connectionName ()).arg (getInfo ().toString ()).arg (db.hostName ()).arg (db.port ());
			std::cout.flush ();

			if (db.open ())
			{
				std::cout << "OK" << std::endl;
				return;
			}
			else
			{
				if (canceled)
				{
					// Failed because canceled
					std::cout << "canceled" << std::endl;
					throw OperationCanceledException ();
				}
				else
				{
					// Failed due to error
					QSqlError error=db.lastError ();
					std::cout << error.databaseText () << std::endl;
					emit databaseError (error.number (), error.databaseText ());

					switch (error.number ())
					{
						case CR_CONN_HOST_ERROR: break; // Retry
						case CR_UNKNOWN_HOST: break; // Retry
						case CR_SERVER_LOST: break; // Retry
						case ER_BAD_DB_ERROR: throw DatabaseDoesNotExistException (error);
						case ER_ACCESS_DENIED_ERROR: throw AccessDeniedException (error);
						case ER_DBACCESS_DENIED_ERROR: throw AccessDeniedException (error);
						default: throw ConnectionFailedException (error);
					}

					// Note that if the operation is canceled during this sleep
					// call, it will only end after the sleep has finished.
					// This is acceptable for a delay as short as 1 second. It
					// could be circumvented by using a QWaitCondition.
					sleep (1);
					std::cout << "Retrying...";
				}
			}
		}
	}

	void DefaultInterface::close ()
	{
		std::cout << "Closing connection" << std::endl;

		db.close ();
	}

	QSqlError DefaultInterface::lastError () const
	{
		return db.lastError ();
	}

	/**
	 * This method is thread safe.
	 */
	void DefaultInterface::cancelConnection ()
	{
		// Set the flag before calling close, because otherwise, there would be
		// a race condition if the blocking call returns with the canceled flag
		// not yet set.
		canceled=true;
		proxy->close ();
	}

	// ******************
	// ** Transactions **
	// ******************

	void DefaultInterface::transaction ()
	{
		// Do not use the QSqlDatabase transaction methods because executeQuery
		// already handles reconnecting and errors.
		// It is possible that this will have to be changed to use the
		// QSqlDatabase methods in the future. In that case, we have to handle
		// errros and reconnect here (see #executeQueryImpl).
		// For the time being, this could also be implemented in Interface, but
		// if it is changed, this is no longer possible. Also, we already have
		// the corresponding methods in ThreadSafeDatabase).

		// Use BEGIN rather than BEGIN WORK or BEGIN TRANSACTION because that
		// is understood by both MySQL and SQLite.
		executeQuery ("BEGIN");
	}

	void DefaultInterface::commit ()
	{
		// See #transaction
		executeQuery ("COMMIT");
	}

	void DefaultInterface::rollback ()
	{
		// See #transaction
		executeQuery ("ROLLBACK");
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

	bool DefaultInterface::retryOnQueryError (int number)
	{
		switch (number)
		{
			case CR_SERVER_GONE_ERROR: return true;
			case CR_SERVER_LOST: return true;
			default: return false;
		}
	}


	QSqlQuery DefaultInterface::executeQueryImpl (const Query &query, bool forwardOnly)
	{
		// Reset the canceled flag; make sure this is always done before
		// entering doExecuteQuery.
		canceled=0;

		while (true)
		{
			try
			{
				if (!db.isOpen ()) openImpl ();

				return doExecuteQuery (query, forwardOnly);
			}
			catch (QueryFailedException &ex)
			{
				if (!retryOnQueryError (ex.error.number ())) throw;

				close ();

				if (opts.display_queries) std::cout << "Retrying...";
			}
		}
	}

	/**
	 * Executes a query and returns the QSqlQuery
	 *
	 * This method is blocking, but can be canceled by calling cancelConnection
	 * from another thread. Unlike #open (and open may be changed), this method
	 * does not use a monitor for a canceled check. This is because
	 *   - the canceled check would be the only reason for the monitor (this
	 *     method does not report progress)
	 *   - passing a monitor here would require passing it through every single
	 *     method that somehow ends up calling this, including most of the
	 *     methods of Interface (and all implementations). Since the
	 *     OperationMonitorInterface would have to be copied by every call,
	 *     this is probably faster then with a monitor
	 * See also the OperationMonitorInterface class.
	 *
	 * @throw QueryFailedException if the query fails
	 * @throw OperationMonitor::CanceledException if cancelConnection was called
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

		emit executingQuery (query);

		if (!query.prepare (sqlQuery))
		{
			if (canceled)
			{
				if (opts.display_queries)
					std::cout << "canceled" << std::endl;
				throw OperationCanceledException ();
			}
			else
			{
				QSqlError error=sqlQuery.lastError ();
				if (opts.display_queries)
					std::cout << error.databaseText () << std::endl;
				emit databaseError (error.number (), error.databaseText ());

				throw QueryFailedException::prepare (error, query);
			}
		}

		query.bindTo (sqlQuery);

		if (opts.display_queries)
			std::cout << "..."; std::cout.flush ();

		if (!sqlQuery.exec ())
		{
			if (canceled)
			{
				if (opts.display_queries)
					std::cout << "canceled" << std::endl;
				throw OperationCanceledException ();
			}
			else
			{
				QSqlError error=sqlQuery.lastError ();
				if (opts.display_queries)
					std::cout << error.databaseText () << std::endl;
				emit databaseError (error.number (), error.databaseText ());

				throw QueryFailedException::execute (error, query);
			}
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
