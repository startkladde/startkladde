#include "OperationMonitor.h"

#include "src/concurrent/synchronized.h"

/*
 * FIXME:
 *   - make thread safe
 */

/*
 * TODO:
 *   - allow suboperations
 *   - documentation
 *
 * Improvements:
 *   - integrate Monitor with Returner so we only need to pass one object
 *     - NB: returner is a template, and we want to emit (progress, status) and
 *       receive (cancel) signals
 *     - Maybe use a Listener
 *     - Maybe use a QVariant
 *     - Look at QFuture
 *   - make Returner assignable so we can write Retuner returner=operation ();
 *     and return operation ().returnedValue (); (?)
 *   - ThreadSafeInterface open vs. asyncOpen: could have this for all methods
 *     -> merge somehow? (e. g. pass Monitor::Synchronous ())
 *   - Use a monitor with a method that does not take a monitor?
 *   - The block for calling a background operation is quite long. Better would
 *     be something like:
 *     Monitor<T> monitor=asyncOperation (params...);
 *     monitor.getResult (); // waits and rethrows
 *     monitor.wait (); // rethrows
 *     For specifying different kinds of monitors, we'll probably need
 *     asyncOperation (new SignalMonitor<T> (), params...);
 *   - Allow canceling before the operation has started; but beware the race
 *     condition that may occur if the canceled flag is set right after it has
 *     been reset and thus prematurely cancels the next operation
 *   - Better integration with cancelConnection of a proxy; currently, the
 *     class using the proxy has to be connected manually
 *   - OperationMonitor should be copied so we can call a method with a monitor
 *     and then discard the monitor instance
 *   - Storing and getting of status, progress etc. should be in the base class
 *     (or at least a DefaultOperationMonitor)
 *   - Can we have an "AsynchronousInterface" which inherites Interface and
 *     stores a Monitor? What about return values?
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

// TODO use it (but it will be ambiguous with Interface::Interface(Monitor)
OperationMonitor::operator OperationMonitorInterface ()
{
	return interface ();
}

void OperationMonitor::cancel ()
{
	synchronized (mutex)
		canceled=true;
}

bool OperationMonitor::isCanceled ()
{
	synchronizedReturn (mutex, canceled);
}
