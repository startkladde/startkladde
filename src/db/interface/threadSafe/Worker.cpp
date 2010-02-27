#include "Worker.h"

// Not explicitly used, but required for destruction of the shared pointer in
// #executeQueryResult
#include "src/db/result/Result.h"
#include "src/db/result/CopiedResult.h"
#include "src/concurrent/Waiter.h"
#include "src/db/Query.h"
#include "src/db/interface/DefaultInterface.h"

namespace Db { namespace Interface { namespace ThreadSafe
{
	// FIXME: error reporting

	// ******************
	// ** Construction **
	// ******************

	Worker::Worker (const DatabaseInfo &dbInfo):
		interface (new DefaultInterface (dbInfo))
	{
	}

	Worker::~Worker ()
	{
		delete interface;
	}


	// ***************************
	// ** Connection management **
	// ***************************

	void Worker::open (Waiter *waiter, bool *result)
	{
		if (result) *result=interface->open ();
		if (waiter) waiter->notify ();
	}

	void Worker::close (Waiter *waiter)
	{
		interface->close ();
		if (waiter) waiter->notify ();
	}

	void Worker::lastError (Waiter *waiter, QSqlError *result) const
	{
		if (result) *result=interface->lastError ();
		if (waiter) waiter->notify ();
	}


	// ******************
	// ** Transactions **
	// ******************

	void Worker::transaction (Waiter *waiter, bool *result)
	{
		if (result) *result=interface->transaction ();
		if (waiter) waiter->notify ();
	}

	void Worker::commit (Waiter *waiter, bool *result)
	{
		if (result) *result=interface->commit ();
		if (waiter) waiter->notify ();
	}

	void Worker::rollback (Waiter *waiter, bool *result)
	{
		if (result) *result=interface->rollback ();
		if (waiter) waiter->notify ();
	}


	// *************
	// ** Queries **
	// *************

	void Worker::executeQuery (Waiter *waiter, Query query)
	{
		interface->executeQuery (query);
		if (waiter) waiter->notify ();
	}

	void Worker::executeQueryResult (Waiter *waiter, QSharedPointer<Result::Result> *result, Query query, bool forwardOnly)
	{
		// Option 1: copy the DefaultResult (is it allowed to access the
		// QSqlQuery from the other thread? It seems to work.)
//		if (result) *result=interface->executeQueryResult (query, forwardOnly);

		// Option 2: create a CopiedResult
		if (result) *result=QSharedPointer<Result::Result> (
			new Result::CopiedResult (
				*interface->executeQueryResult (query, forwardOnly)
			)
		);

		if (waiter) waiter->notify ();
	}

	void Worker::queryHasResult (Waiter *waiter, bool *result, Query query)
	{
		if (result) *result=interface->queryHasResult (query);
		if (waiter) waiter->notify ();
	}

} } }
