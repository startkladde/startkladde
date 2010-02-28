#include "ThreadSafeInterface.h"

#include <iostream> // remove

#include <QSqlError>

#include "src/db/result/Result.h"
#include "src/concurrent/Waiter.h"

namespace Db { namespace Interface { namespace ThreadSafe
{
	// ******************
	// ** Construction **
	// ******************

	ThreadSafeInterface::ThreadSafeInterface (const DatabaseInfo &info):
		Interface (info), thread (info)
	{
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
		Waiter waiter;
		bool result;

		thread.open (&waiter, &result);
		waiter.wait ();

		return result;
	}

	void ThreadSafeInterface::close ()
	{
		Waiter waiter;

		thread.close (&waiter);
		waiter.wait ();
	}

	QSqlError ThreadSafeInterface::lastError () const
	{
		Waiter waiter;
		QSqlError result;

		thread.lastError (&waiter, &result);
		waiter.wait ();

		return result;
	}


	// ******************
	// ** Transactions **
	// ******************

	bool ThreadSafeInterface::transaction ()
	{
		Waiter waiter;
		bool result;

		thread.transaction (&waiter, &result);
		waiter.wait ();

		return result;
	}

	bool ThreadSafeInterface::commit ()
	{
		Waiter waiter;
		bool result;

		thread.commit (&waiter, &result);
		waiter.wait ();

		return result;
	}

	bool ThreadSafeInterface::rollback ()
	{
		Waiter waiter;
		bool result;

		thread.rollback (&waiter, &result);
		waiter.wait ();

		return result;
	}


	// *************
	// ** Queries **
	// *************

	void ThreadSafeInterface::executeQuery (const Query &query)
	{
		Waiter waiter;

		thread.executeQuery (&waiter, query);
		waiter.wait ();
	}

	QSharedPointer<Result::Result> ThreadSafeInterface::executeQueryResult (const Query &query, bool forwardOnly)
	{
		Waiter waiter;
		QSharedPointer<Result::Result> result;

		thread.executeQueryResult (&waiter, &result, query, forwardOnly);
		waiter.wait ();

		return result;
	}

	bool ThreadSafeInterface::queryHasResult (const Query &query)
	{
		Waiter waiter;
		bool result;

		thread.queryHasResult (&waiter, &result, query);
		waiter.wait ();

		return result;
	}

} } }
