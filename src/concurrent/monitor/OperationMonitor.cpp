#include "OperationMonitor.h"

#include <QAtomicInt>

#include "src/concurrent/synchronized.h"


/*
 * TODO:
 *   - implement RAII so finish is always signaled?
 *   - allow suboperations
 *   - integrate Monitor with Returner so we only need to pass one object
 *     - NB: returner is a template, and we want to emit (progress, status) and
 *       receive (cancel) signals
 *     - Maybe use a Listener
 *     - Look at QFuture
 *   - make Returner copyable so we can write Retuner returner=operation ()...
 *
 *
 * In actual class:
 *   if (monitor) monitor->setStatus ("...");
 *   if (monitor) monitor->setProgress (22, 100);
 *
 * Call without monitor:
 *   migrator->migrate (); // default is NULL
 *
 * Normal call:
 *   PrintingOperationMonitor monitor;
 *   migrator->migrate (monitor.interface ());
 *
 * Async call, in worker:
 *   OperationMonitor *monitor;
 *   returnOrException (migrator->migrate (monitor.interface ()));
 */


// **********************
// ** OperationMonitor **
// **********************

OperationMonitor::OperationMonitor ():
	theInterface (this), canceled (false)
{
}

OperationMonitor::~OperationMonitor ()
{
}

OperationMonitor::Interface OperationMonitor::interface ()
{
	// Make a copy of the interface
	return theInterface;
}

//virtual operator Interface ()
//{
//	return interface ();
//}

void OperationMonitor::cancel ()
{
	synchronized (mutex)
		canceled=true;
}

bool OperationMonitor::isCanceled ()
{
	synchronizedReturn (mutex, canceled);
}

// *********************************
// ** OperationMonitor::Interface **
// *********************************

OperationMonitor::Interface::~Interface ()
{
	int newRefCount=refCount->fetchAndAddOrdered (-1)-1;

	if (newRefCount==1)
		monitor->setEnded ();
	else if (newRefCount==0)
		delete refCount;
}

OperationMonitor::Interface::Interface (const OperationMonitor::Interface &other):
	monitor (other.monitor), refCount (other.refCount)
{
	// FIXME: this is very similar to QFutureInterfaceBase, but isn't there a
	// race condition in case the last instance is destroyed at this point?
	other.refCount->ref ();
}


OperationMonitor::Interface &OperationMonitor::Interface::operator= (const OperationMonitor::Interface &other)
{
	if (&other==this) return *this;

	// FIXME: this is very similar to QFutureInterfaceBase, but isn't there a
	// race condition in case the last instance is destroyed at this point?
	other.refCount->ref ();

	int newRefCount=refCount->fetchAndAddOrdered (-1)-1;

	if (newRefCount==1)
		monitor->setEnded ();
	else if (newRefCount==0)
		delete refCount;

	refCount=other.refCount;
	monitor=other.monitor;

	return *this;
}

OperationMonitor::Interface::Interface (OperationMonitor *monitor):
	monitor (monitor), refCount (new QAtomicInt (1))
{

}

void OperationMonitor::Interface::status (const QString &text)
{
	monitor->setStatus (text);
}

void OperationMonitor::Interface::progress (int progress, int maxProgress)
{
	monitor->setProgress (progress, maxProgress);
}

void OperationMonitor::Interface::ended ()
{
	monitor->setEnded ();
}

bool OperationMonitor::Interface::canceled ()
{
	return monitor->isCanceled ();
}

