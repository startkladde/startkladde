#include "OperationMonitor.h"

#include <iostream>

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
	std::cout << "OperationMonitor::cancel ()" << std::endl;

	synchronized (mutex)
		canceled=true;
}

bool OperationMonitor::isCanceled ()
{
	synchronizedReturn (mutex, canceled);
}
