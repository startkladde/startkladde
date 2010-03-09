/*
 * Improvements:
 *   - consider passing a pointer to the query instead of copying the query
 */
#include "ThreadSafeInterface.h"

#include <iostream>

#include <QSqlError>

#include "src/db/result/Result.h"
#include "src/concurrent/Returner.h"
#include "src/db/interface/DefaultInterface.h"
#include "src/db/result/CopiedResult.h"

// ******************
// ** Construction **
// ******************

ThreadSafeInterface::ThreadSafeInterface (const DatabaseInfo &info):
	Interface (info), interface (NULL)
{
	// For connecting the signal, we need to know that it's a
	// DefaultInterface. Afterwards, we assign it to the
	// AbstractInterface *interface. TODO shouldn't the signal be declared
	// in AbstractInterface?
	DefaultInterface *defaultInterface=new DefaultInterface (info);
	connect (defaultInterface, SIGNAL (databaseError (int, QString)), this, SIGNAL (databaseError (int, QString)));
	connect (defaultInterface, SIGNAL (executingQuery (Query)), this, SIGNAL (executingQuery (Query)));
	interface=defaultInterface;

#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
	CONNECT (open      (Returner<bool>      *));
	CONNECT (close     (Returner<void>      *));
	CONNECT (lastError (Returner<QSqlError> *));

	CONNECT (transaction (Returner<void> *));
	CONNECT (commit      (Returner<void> *));
	CONNECT (rollback    (Returner<void> *));

	CONNECT (executeQuery       (Returner<void>                    *, Query));
	CONNECT (executeQueryResult (Returner<QSharedPointer<Result> > *, Query, bool));
	CONNECT (queryHasResult     (Returner<bool>                    *, Query));
#undef CONNECT

	moveToThread (&thread);
	thread.start ();
}

ThreadSafeInterface::~ThreadSafeInterface ()
{
	delete interface;
	thread.quit ();

	std::cout << "Waiting for interface worker thread to terminate..." << std::flush;
	if (thread.wait (1000)) std::cout << "OK"      << std::endl;
	else                    std::cout << "Timeout" << std::endl;
}


// ***********************
// ** Front-end methods **
// ***********************

bool ThreadSafeInterface::open ()
{
	Returner<bool> returner;
	emit sig_open (&returner);
	return returner.returnedValue ();
}

void ThreadSafeInterface::close ()
{
	Returner<void> returner;
	emit sig_close (&returner);
	returner.wait ();
}

QSqlError ThreadSafeInterface::lastError () const
{
	Returner<QSqlError> returner;
	emit sig_lastError (&returner);
	return returner.returnedValue ();
}

void ThreadSafeInterface::transaction ()
{
	Returner<void> returner;
	emit sig_transaction (&returner);
	returner.wait ();
}

void ThreadSafeInterface::commit ()
{
	Returner<void> returner;
	emit sig_commit (&returner);
	returner.wait ();
}

void ThreadSafeInterface::rollback ()
{
	Returner<void> returner;
	emit sig_rollback (&returner);
	returner.wait ();
}

void ThreadSafeInterface::executeQuery (const Query &query)
{
	Returner<void> returner;
	emit sig_executeQuery (&returner, query);
	returner.wait ();
}

QSharedPointer<Result> ThreadSafeInterface::executeQueryResult (const Query &query, bool forwardOnly)
{
	Returner<QSharedPointer<Result> > returner;
	emit sig_executeQueryResult (&returner, query, forwardOnly);
	return returner.returnedValue ();
}

bool ThreadSafeInterface::queryHasResult (const Query &query)
{
	Returner<bool> returner;
	emit sig_queryHasResult (&returner, query);
	return returner.returnedValue ();
}


// ********************
// ** Back-end slots **
// ********************

void ThreadSafeInterface::slot_open (Returner<bool> *returner)
{
	returnOrException (returner, interface->open ());
}

void ThreadSafeInterface::slot_close (Returner<void> *returner)
{
	returnVoidOrException (returner, interface->close ());
}

void ThreadSafeInterface::slot_lastError (Returner<QSqlError> *returner) const
{
	returnOrException (returner, interface->lastError ());
}

void ThreadSafeInterface::slot_transaction (Returner<void> *returner)
{
	returnVoidOrException (returner, interface->transaction ());
}

void ThreadSafeInterface::slot_commit (Returner<void> *returner)
{
	returnVoidOrException (returner, interface->commit ());
}

void ThreadSafeInterface::slot_rollback (Returner<void> *returner)
{
	returnVoidOrException (returner, interface->rollback ());
}

void ThreadSafeInterface::slot_executeQuery (Returner<void> *returner, Query query)
{
	returnVoidOrException (returner, interface->executeQuery (query));
}

void ThreadSafeInterface::slot_executeQueryResult (Returner<QSharedPointer<Result> > *returner, Query query, bool forwardOnly)
{
	// Option 1: copy the DefaultResult (is it allowed to access the
	// QSqlQuery from the other thread? It seems to work.)
//		returnOrException (returner, interface->executeQueryResult (query, forwardOnly));

	// Option 2: create a CopiedResult
	(void)forwardOnly;
	returnOrException (returner, QSharedPointer<Result> (
		new CopiedResult (
			// When copying, we can always set forwardOnly
			*interface->executeQueryResult (query, true)
		)
	));
}

void ThreadSafeInterface::slot_queryHasResult (Returner<bool> *returner, Query query)
{
	returnOrException (returner, interface->queryHasResult (query));
}

// ************
// ** Others **
// ************

/**
 * Called directly, this method is not executed on the background thread.
 * When using as a slot, a DirectConnection should be used or the slot will
 * be invoked in the background thread where it won't be very useful.
 */
void ThreadSafeInterface::cancelConnection ()
{
	interface->cancelConnection ();
}
