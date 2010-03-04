#include "Worker.h"

#include <iostream>

// Not explicitly used, but required for destruction of the shared pointer in
// #executeQueryResult
#include "src/db/result/Result.h"
#include "src/db/result/CopiedResult.h"
#include "src/db/Query.h"
#include "src/db/interface/DefaultInterface.h"
#include "src/concurrent/Returner.h"
#include "src/concurrent/monitor/OperationMonitor.h"
//#include "src/concurrent/monitor/OperationMonitorInterface.h"

namespace Db { namespace Interface { namespace ThreadSafe
{
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

	void Worker::open (Returner<bool> *returner)
	{
		returnOrException (returner, interface->open ());
	}

	void Worker::asyncOpen (Returner<bool> *returner, OperationMonitor *monitor)
	{
		returnOrException (returner, interface->open (monitor->interface ()));
	}

	void Worker::close (Returner<void> *returner)
	{
		returnVoidOrException (returner, interface->close ());
	}

	void Worker::lastError (Returner<QSqlError> *returner) const
	{
		returnOrException (returner, interface->lastError ());
	}


	// ******************
	// ** Transactions **
	// ******************

	void Worker::transaction (Returner<bool> *returner)
	{
		returnOrException (returner, interface->transaction ());
	}

	void Worker::commit (Returner<bool> *returner)
	{
		returnOrException (returner, interface->commit ());
	}

	void Worker::rollback (Returner<bool> *returner)
	{
		returnOrException (returner, interface->rollback ());
	}


	// *************
	// ** Queries **
	// *************

	void Worker::executeQuery (Returner<void> *returner, Db::Query query)
	{
		returnVoidOrException (returner, interface->executeQuery (query));
	}

	void Worker::executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, Db::Query query, bool forwardOnly)
	{
		// Option 1: copy the DefaultResult (is it allowed to access the
		// QSqlQuery from the other thread? It seems to work.)
//		returnOrException (returner, interface->executeQueryResult (query, forwardOnly));

		// Option 2: create a CopiedResult
		(void)forwardOnly;
		returnOrException (returner, QSharedPointer<Result::Result> (
			new Result::CopiedResult (
				// When copying, we can always set forwardOnly
				*interface->executeQueryResult (query, true)
			)
		));
	}

	void Worker::queryHasResult (Returner<bool> *returner, Db::Query query)
	{
		returnOrException (returner, interface->queryHasResult (query));
	}
} } }
