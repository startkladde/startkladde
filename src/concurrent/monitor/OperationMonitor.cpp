#include "OperationMonitor.h"

#include "src/concurrent/synchronized.h"

/*
 * FIXME:
 *   - make thread safe
 */

/*
 * TODO:
 *   - allow suboperations
 *   - allow calling an operation without monitor (nullobject interface?)
 *   - integrate Monitor with Returner so we only need to pass one object
 *     - NB: returner is a template, and we want to emit (progress, status) and
 *       receive (cancel) signals
 *     - Maybe use a Listener
 *     - Look at QFuture
 *   - make Returner copyable so we can write Retuner returner=operation ();
 *     and return operation ().returnedValue ();
 *   - documentation
 *
 *
 * In actual class:
 *   if (monitor) monitor->setStatus ("...");
 *   if (monitor) monitor->setProgress (22, 100);
 */


OperationMonitor::OperationMonitor ():
	theInterface (this), canceled (false)
{
}

OperationMonitor::~OperationMonitor ()
{
}

OperationMonitorInterface OperationMonitor::interface ()
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

