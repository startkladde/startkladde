#include "DbManager.h"

#include <QObject>
#include <QInputDialog>

// TODO some of these can probably be removed from MainWindow and others
#include "src/concurrent/monitor/SignalOperationMonitor.h"
#include "src/gui/windows/MonitorDialog.h"
#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/db/interface/DefaultInterface.h"
#include "src/db/migration/Migrator.h"
#include "src/db/interface/exceptions/SqlException.h"
#include "src/db/interface/exceptions/AccessDeniedException.h"
#include "src/db/interface/exceptions/DatabaseDoesNotExistException.h"
#include "src/concurrent/monitor/OperationCanceledException.h"

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

void DbManager::confirmOrCancel (const QString &title, const QString &question, QWidget *parent)
{
	if (!yesNoQuestion (parent, title, question))
		throw ConnectCanceledException ();
}

void DbManager::grantPermissions (QWidget *parent)
{
	DatabaseInfo info=interface.getInfo ();

	// Get the root password from the user
	QString title=utf8 ("Datenbank-Passwort benötigt");
	QString text=utf8 (
		"Der Datenbankbenutzer %1 existiert nicht, das angegebene Passwort\n"
		"stimmt nicht oder der Benutzer hat unzureichende Zugriffsrechte auf\n"
		"die Datenbank %2. Zur automatischen Korrektur wird das Passwort des\n"
		"Datenbankbenutzers root benötigt.\n"
		"Bitte das Kennwort für root eingeben:")
		.arg (info.username, info.database);

	bool retry=true;
	while (retry)
	{
		retry=false;

		bool ok;
		QString rootPassword=QInputDialog::getText (parent,
			title, text, QLineEdit::Password, "", &ok);

		if (!ok)
			throw DbManager::ConnectCanceledException ();

		DatabaseInfo rootInfo=info;
		rootInfo.database="";
		rootInfo.username="root";
		rootInfo.password=rootPassword;

		try
		{
			Db::Interface::DefaultInterface rootInterface (rootInfo);
			rootInterface.open ();
			rootInterface.grantAll (info.database, info.username, info.password);
			rootInterface.close ();
		}
		catch (Db::Interface::AccessDeniedException) // Actually: only 1045
		{
			text=utf8 (
				"Anmeldung als root verweigert. Möglicherweise ist das\n"
				"angegebene Kennwort nicht richtig.\n"
				"Bitte das Kennwort für root eingeben:");
			retry=true;
		}
	}
}

void DbManager::createDatabase (QWidget *parent)
{
	const DatabaseInfo &info=interface.getInfo ();

	DatabaseInfo createInfo=info;
	createInfo.database="";
	// FIXME background
	Db::Interface::DefaultInterface createInterface (createInfo);

	try
	{
		createInterface.createDatabase (info.database);
	}
	catch (...)
	{
		// FIXME required?
		createInterface.close ();
		throw;
	}
}


void DbManager::checkVersion (QWidget *parent)
{
	if (isEmpty (parent))
	{
		confirmOrCancel ("Datenbank leer",
			"Die Datenbank ist leer. Soll sie jetzt erstellt werden?", parent);

		Returner<void> returner;
		SignalOperationMonitor monitor;
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
		migrator.loadSchema (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait (); // Required so any exceptions are rethrown

		// After loading the schema, the database must be current.
		// TODO different message if canceled
		ensureCurrent ("Datenbank ist nach Erstellen nicht aktuell.", parent);
	}
	else if (!isCurrent (parent))
	{
		// TODO try to reuse monitor and dialog
		Returner<quint64> currentVersionReturner;
		SignalOperationMonitor currentVersionMonitor;
		QObject::connect (&currentVersionMonitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
		migrator.currentVersion (currentVersionReturner, currentVersionMonitor);
		MonitorDialog::monitor (currentVersionMonitor, "Verbindungsaufbau", parent);
		quint64 currentVersion=currentVersionReturner.returnedValue ();

		quint64 latestVersion=Migrator::latestVersion ();

		Returner<QList<quint64> > pendingMigrationsReturner;
		SignalOperationMonitor pendingMigrationsMonitor;
		QObject::connect (&pendingMigrationsMonitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
		migrator.pendingMigrations (pendingMigrationsReturner, pendingMigrationsMonitor);
		MonitorDialog::monitor (pendingMigrationsMonitor, "Verbindungsaufbau", parent);
		quint64 numPendingMigrations=pendingMigrationsReturner.returnedValue ().size ();

		confirmOrCancel ("Datenbank nicht aktuell", utf8 (
				"Die Datenbank ist nicht aktuell:\n"
				"  - Momentane Version: %1\n"
				"  - Aktuelle Version: %2\n"
				"  - Anzahl ausstehender Migrationen: %3\n"
				"\n"
				"Achtung: die Aktualisierung sollte nicht unterbrochen werden, da dies zu einer inkonsistenten Datenbank führen kann, die nicht automatisch repariert werden kann.\n"
				"Vor dem Aktualisieren der Datenbank sollte eine Sicherungskopie der Datenbank erstellt werden.\n"
				"\n"
				"Soll die Datenbank jetzt aktualisiert werden?"
			).arg (currentVersion).arg (latestVersion).arg (numPendingMigrations), parent);

		Returner<void> returner;
		SignalOperationMonitor monitor;
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
		migrator.migrate (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait (); // Required so any exceptions are rethrown

		// After migrating, the database must be current.
		// TODO different message if canceled
		ensureCurrent ("Datenbank ist nach der Aktualisierung nicht aktuell.", parent);
	}
}

void DbManager::openInterface (QWidget *parent)
{
	// Open the interface. This would also be done automatically, but this
	// allows us to detect a missing database before checking the version.
	try
	{
		Returner<bool> returner;
		SignalOperationMonitor monitor;
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
		interface.asyncOpen (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait ();
	}
	catch (Db::Interface::DatabaseDoesNotExistException)
	{
		// The database does not exist. We have some permissions on the
		// database, or we would get "access denied". Try to create it, and if
		// that particular permission is missing, the enclosing routine will
		// have to pick up and grant us the permissions.

		confirmOrCancel ("Datenbank erstellen?", QString (
			"Die Datenbank %1 existiert nicht. Soll sie erstellt werden?")
			.arg (interface.getInfo ().database), parent);

		// Create the database, which involves opening a connection without a
		// default database
		createDatabase (parent);

		// Since creating the database succeeded, we should now be able to open
		// it and load the schema.
		interface.open (); // FIXME background

		Returner<void> returner;
		SignalOperationMonitor monitor;
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()));
		migrator.loadSchema (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait (); // Required so any exceptions are rethrown

		// After loading the schema, the database must be current.
		// TODO different message if canceled
		ensureCurrent ("Nach dem Laden ist die Datenbank nicht aktuell.", parent);
	}
}

void DbManager::connectImpl (QWidget *parent)
{
	try
	{
		openInterface (parent);
		checkVersion (parent);

		clearCache ();
		refreshCache (parent);

		// FIXME need this in MainWindow
//		refreshFlights ();
//		setDatabaseActionsEnabled (true);
	}
	catch (...)
	{
		// TODO check it works even if it's not open
		interface.close ();
		throw;
	}
}

void DbManager::connect (QWidget *parent)
{
	try
	{
		try
		{
			connectImpl (parent);
		}
		catch (Db::Interface::AccessDeniedException)
		{
			grantPermissions (parent);
			connectImpl (parent);
		}
		// TODO also for access denied during query (1142)
	}
	catch (DbManager::ConnectCanceledException)
	{
		showWarning ("Verbindungsaufbau abgebrochen",
			"Der Verbindungsaufbau wurde abgebrochen",
			parent);
	}
	catch (OperationCanceledException &ex)
	{
		showWarning ("Verbindungsaufbau abgebrochen",
			"Der Verbindungsaufbau wurde abgebrochen",
			parent);
	}
	catch (ConnectFailedException &ex)
	{
		showWarning ("Verbindungsaufbau fehlgeschlagen",
			QString ("Beim Verbindungsaufbau ist ein Fehler aufgetreten: %1").arg (ex.message),
			parent);
	}
	catch (Db::Interface::SqlException &ex)
	{
		QSqlError error=ex.error;

		QString text=QString (
			"Beim Verbindungsaufbau ist ein Fehler aufgetreten: %1"
			" (Fehlercode %2, Typ %3)"
			).arg (error.databaseText ()).arg (error.number ()).arg (error.type ());
		showWarning ("Fehler beim Verbindungsaufbau", text, parent);
	}
}



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

