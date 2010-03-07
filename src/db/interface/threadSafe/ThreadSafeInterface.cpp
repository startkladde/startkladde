/*
 * Improvements:
 *   - move asyncOpen to an interfaceWorker, we don't usually need to execute
 *     interface operations asynchronously because they are already called from
 *     an ansynchronous operation (like dbWorker, migrationWorker)
 */
#include "ThreadSafeInterface.h"

#include <iostream>

#include <QSqlError>

#include "src/db/result/Result.h"
#include "src/concurrent/Returner.h"

namespace Db { namespace Interface { namespace ThreadSafe
{
	// ******************
	// ** Construction **
	// ******************

	ThreadSafeInterface::ThreadSafeInterface (const DatabaseInfo &info):
		Interface (info), thread (info)
	{
		connect (&thread, SIGNAL (databaseError (int, QString)), this, SIGNAL (databaseError (int, QString)));
		thread.start ();

		// Wait for the thread to connect the signals to the worker
		thread.waitStartup ();
	}

	ThreadSafeInterface::~ThreadSafeInterface ()
	{
		// Terminete the thread's event loop with the requestedExit exit code
		// so it doesn't print a message.
		thread.exit (Thread::requestedExit);

		std::cout << "Waiting for database interface thread to terminate...";
		std::cout.flush ();

		if (thread.wait (1000))
			std::cout << "OK" << std::endl;
		else
			std::cout << "Timeout" << std::endl;
	}


	// ***************************
	// ** Connection management **
	// ***************************

	bool ThreadSafeInterface::open ()
	{
		Returner<bool> returner;
		thread.open (&returner);
		return returner.returnedValue ();
	}

	void ThreadSafeInterface::asyncOpen (Returner<bool> &returner, OperationMonitor &monitor)
	{
		thread.asyncOpen (&returner, &monitor);
	}

	void ThreadSafeInterface::close ()
	{
		Returner<void> returner;
		thread.close (&returner);
		returner.wait ();
	}

	QSqlError ThreadSafeInterface::lastError () const
	{
		Returner<QSqlError> returner;
		thread.lastError (&returner);
		return returner.returnedValue ();
	}

	void ThreadSafeInterface::cancelConnection ()
	{
		thread.cancelConnection ();
	}


	// ******************
	// ** Transactions **
	// ******************

	void ThreadSafeInterface::transaction ()
	{
		Returner<void> returner;
		thread.transaction (&returner);
		returner.wait ();
	}

	void ThreadSafeInterface::commit ()
	{
		Returner<void> returner;
		thread.commit (&returner);
		returner.wait ();
	}

	void ThreadSafeInterface::rollback ()
	{
		Returner<void> returner;
		thread.rollback (&returner);
		returner.wait ();
	}


	// *************
	// ** Queries **
	// *************

	void ThreadSafeInterface::executeQuery (const Query &query)
	{
		Returner<void> returner;
		thread.executeQuery (&returner, query);
		returner.wait ();
	}

	QSharedPointer<Result::Result> ThreadSafeInterface::executeQueryResult (const Query &query, bool forwardOnly)
	{
		Returner<QSharedPointer<Result::Result> > returner;
		thread.executeQueryResult (&returner, query, forwardOnly);
		return returner.returnedValue ();
	}

	bool ThreadSafeInterface::queryHasResult (const Query &query)
	{
		Returner<bool> returner;
		thread.queryHasResult (&returner, query);
		return returner.returnedValue ();
	}

} } }
