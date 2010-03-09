/*
 * Improvements:
 *   - the thread created by this worker is not really necessary as the
 *     ThreadSafeInterface already has a background thread (but with blocking
 *     calls)
 */
#include "InterfaceWorker.h"

#include <iostream>

#include "src/db/interface/ThreadSafeInterface.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/concurrent/Returner.h"

InterfaceWorker::InterfaceWorker (ThreadSafeInterface &interface):
	interface (interface)
{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
	CONNECT (open           (Returner<bool> *, OperationMonitor *));
	CONNECT (createDatabase (Returner<void> *, OperationMonitor *, QString, bool));
	CONNECT (grantAll       (Returner<void> *, OperationMonitor *, QString, QString, QString));
#undef CONNECT

	moveToThread (&thread);
	thread.start ();
}

InterfaceWorker::~InterfaceWorker ()
{
	thread.quit ();

	std::cout << "Waiting for interface worker thread to terminate..." << std::flush;
	if (thread.wait (1000)) std::cout << "OK"      << std::endl;
	else                    std::cout << "Timeout" << std::endl;
}


// ***********************
// ** Front-end methods **
// ***********************

void InterfaceWorker::open (Returner<bool> &returner, OperationMonitor &monitor)
{
	emit sig_open (&returner, &monitor);
}

void InterfaceWorker::createDatabase (Returner<void> &returner, OperationMonitor &monitor, const QString &name, bool skipIfExists)
{
	emit sig_createDatabase (&returner, &monitor, name, skipIfExists);
}

void InterfaceWorker::grantAll (Returner<void> &returner, OperationMonitor &monitor, const QString &database, const QString &username, const QString &password)
{
	emit sig_grantAll (&returner, &monitor, database, username, password);
}


// ********************
// ** Back-end slots **
// ********************

void InterfaceWorker::slot_open (Returner<bool> *returner, OperationMonitor *monitor)
{
	OperationMonitorInterface monitorInterface=monitor->interface ();
	monitorInterface.status ("Verbindung herstellen");
	returnOrException (returner, interface.open ());
}

void InterfaceWorker::slot_createDatabase (Returner<void> *returner, OperationMonitor *monitor, QString name, bool skipIfExists)
{
	OperationMonitorInterface monitorInterface=monitor->interface ();
	returnVoidOrException (returner, interface.createDatabase (name, skipIfExists));
}

void InterfaceWorker::slot_grantAll (Returner<void> *returner, OperationMonitor *monitor, QString database, QString username, QString password)
{
	OperationMonitorInterface monitorInterface=monitor->interface ();
	returnVoidOrException (returner, interface.grantAll (database, username, password));
}
