#include "DbManager.h"

#include <QObject>

// TODO some of these can probably be removed from MainWindow and others
#include "src/concurrent/monitor/SignalOperationMonitor.h"
#include "src/gui/windows/MonitorDialog.h"
#include "src/util/qString.h"

DbManager::DbManager (const DatabaseInfo &info):
	interface (info), db (interface), cache (db),
	dbWorker (db), migrator (interface), cacheThread (cache)
{
}

DbManager::DbManager (const DbManager &other):
	interface (other.interface.getInfo ()), db (interface), cache (db),
	dbWorker (db), migrator (interface), cacheThread (cache)
{
	assert (!"DbManager copied");
}

DbManager &DbManager::operator= (const DbManager &other)
{
	(void)other;
	assert (!"DbManager copied");
}

DbManager::~DbManager ()
{
}


// ***********************
// ** Schema management **
// ***********************

bool DbManager::isCurrent (QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
	migrator.isCurrent (returner, monitor);
	MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
	return returner.returnedValue ();
}

void DbManager::ensureCurrent (const QString &message, QWidget *parent)
{
	if (!isCurrent (parent))
		throw ConnectFailedException (message);
}

bool DbManager::isEmpty (QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
	migrator.isEmpty (returner, monitor);
	MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
	return returner.returnedValue ();
}

// ***************************
// ** Connection management **
// ***************************




// *********************
// ** Data management **
// *********************

void DbManager::clearCache ()
{
	cache.clear ();
}

void DbManager::refreshCache (QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
	cacheThread.refreshAll (returner, monitor);
	MonitorDialog::monitor (monitor, "Daten abrufen", parent);
	returner.wait ();
}

void DbManager::fetchFlights (QDate date, QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
	cacheThread.fetchFlightsOther (returner, monitor, date);
	MonitorDialog::monitor (monitor, utf8 ("Flüge abrufen"), parent);
	returner.wait ();
}

