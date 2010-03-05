/*
 * Return Thread to InterfaceThread, Worker to InterfaceWorker
 * Consider merging ThreadSafeInterface and InterfaceThread: emit directly
 */
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

		CONNECT (open      (Returner<bool>      *));
		CONNECT (asyncOpen (Returner<bool>      *, OperationMonitor *));
		CONNECT (close     (Returner<void>      *));
		CONNECT (lastError (Returner<QSqlError> *));

		CONNECT (transaction (Returner<bool> *));
		CONNECT (commit      (Returner<bool> *));
		CONNECT (rollback    (Returner<bool> *));

		CONNECT (executeQuery       (Returner<void>                            *, Db::Query));
		CONNECT (executeQueryResult (Returner<QSharedPointer<Result::Result> > *, Db::Query, bool));
		CONNECT (queryHasResult     (Returner<bool>                            *, Db::Query));

#undef CONNECT

		// Note that if a signal is sent at this point, it will be queued even
		// though the event loop is not running yet, so we can notify those
		// waiting for our startup.
		startupWaiter.notify ();

		// Run the event loop, delivering the signals connected above to the
		// worker
		int result=exec ();

		if (result!=requestedExit)
			std::cout << "Db::Interface::ThreadSafe::Thread exited" << std::endl;
	}

	void Thread::waitStartup ()
	{
		startupWaiter.wait ();
	}


	// ***************************
	// ** Connection management **
	// ***************************

	void Thread::open (Returner<bool> *returner)
	{
		emit sig_open (returner);
	}

	void Thread::asyncOpen (Returner<bool> *returner, OperationMonitor *monitor)
	{
		emit sig_asyncOpen (returner, monitor);
	}

	void Thread::close (Returner<void> *returner)
	{
		emit sig_close (returner);
	}

	void Thread::lastError (Returner<QSqlError> *returner) const
	{
		emit sig_lastError (returner);
	}


	// ******************
	// ** Transactions **
	// ******************

	void Thread::transaction (Returner<bool> *returner) { emit sig_transaction (returner); }
	void Thread::commit      (Returner<bool> *returner) { emit sig_commit      (returner); }
	void Thread::rollback    (Returner<bool> *returner) { emit sig_rollback    (returner); }


	// *************
	// ** Queries **
	// *************

	void Thread::executeQuery (Returner<void> *returner, const Query &query)
	{
		emit sig_executeQuery (returner, query);
	}

	void Thread::executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, const Query &query, bool forwardOnly)
	{
		emit sig_executeQueryResult (returner, query, forwardOnly);
	}

	void Thread::queryHasResult (Returner<bool> *returner, const Query &query)
	{
		emit sig_queryHasResult (returner, query);
	}

} } }
