#include "Thread.h"

#include <iostream>

namespace Db { namespace Interface { namespace ThreadSafe
{
	Thread::Thread (const DatabaseInfo &dbInfo):
		worker (NULL), dbInfo (dbInfo)
	{
	}

	Thread::~Thread ()
	{
	}

	void Thread::run ()
	{
		worker=new Worker (dbInfo);

#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), worker, SLOT (definition))

		CONNECT (open (Waiter *, bool *));
		CONNECT (close (Waiter *));
		CONNECT (lastError (Waiter *, QSqlError *));

		CONNECT (transaction (Waiter *, bool *));
		CONNECT (commit      (Waiter *, bool *));
		CONNECT (rollback    (Waiter *, bool *));

		CONNECT (executeQuery       (Waiter *,                                   Db::Query));
		CONNECT (executeQueryResult (Waiter *, QSharedPointer<Result::Result> *, Db::Query, bool));
		CONNECT (queryHasResult     (Waiter *, bool                           *, Db::Query));

#undef CONNECT

		// Note that if a signal is sent at this point, it will be queued even
		// though the event loop is not running yet, so we can notify those
		// waiting for our startup.
		startupWaiter.notify ();

		// Run the event loop, delivering the signals connected above to the
		// worker
		exec ();

		std::cout << "Db::Interface::ThreadSafe::Thread exited" << std::endl;
	}

	void Thread::waitStartup ()
	{
		startupWaiter.wait ();
	}


	// ***************************
	// ** Connection management **
	// ***************************

	void Thread::open (Waiter *waiter, bool *result)
	{
		std::cout << "emit sig_open" << std::endl;

		emit sig_open (waiter, result);
	}

	void Thread::close (Waiter *waiter)
	{
		emit sig_close (waiter);
	}

	void Thread::lastError (Waiter *waiter, QSqlError *result) const
	{
		emit sig_lastError (waiter, result);
	}


	// ******************
	// ** Transactions **
	// ******************

	void Thread::transaction (Waiter *waiter, bool *result) { emit sig_transaction (waiter, result); }
	void Thread::commit      (Waiter *waiter, bool *result) { emit sig_commit      (waiter, result); }
	void Thread::rollback    (Waiter *waiter, bool *result) { emit sig_rollback    (waiter, result); }


	// *************
	// ** Queries **
	// *************

	void Thread::executeQuery (Waiter *waiter, const Query &query)
	{
		emit sig_executeQuery (waiter, query);
	}

	void Thread::executeQueryResult (Waiter *waiter, QSharedPointer<Result::Result> *result, const Query &query, bool forwardOnly)
	{
		emit sig_executeQueryResult (waiter, result, query, forwardOnly);
	}

	void Thread::queryHasResult (Waiter *waiter, bool *result, const Query &query)
	{
		emit sig_queryHasResult (waiter, result, query);
	}



} } }
