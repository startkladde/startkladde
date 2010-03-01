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


	// ******************
	// ** Transactions **
	// ******************

	bool ThreadSafeInterface::transaction ()
	{
		Returner<bool> returner;
		thread.transaction (&returner);
		return returner.returnedValue ();
	}

	bool ThreadSafeInterface::commit ()
	{
		Returner<bool> returner;
		thread.commit (&returner);
		return returner.returnedValue ();
	}

	bool ThreadSafeInterface::rollback ()
	{
		Returner<bool> returner;
		thread.rollback (&returner);
		return returner.returnedValue ();
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
