/*
 * Improvements:
 *   - have the whole creation procress run in the background. This would
 *     remove a lot of monitor blocks (reducing code duplication) and interface
 *     worker (thread) creations and make it
 *     easier to leave the dialog open and provide progress feedback. But this
 *     requires getting user input from a background thread.
 */
#include "DbManager.h"

#include <QObject>
#include <QInputDialog>

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
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"

#include "src/concurrent/DefaultQThread.h" //remove

DbManager::DbManager (const DatabaseInfo &info):
	interface (info), db (interface), cache (db),
	interfaceWorker (interface), dbWorker (db), migratorWorker (interface), cacheWorker (cache)
{
}

DbManager::DbManager (const DbManager &other):
	interface (other.interface.getInfo ()), db (interface), cache (db),
	interfaceWorker (interface), dbWorker (db), migratorWorker (interface), cacheWorker (cache)
{
	assert (!"DbManager copied");
}

DbManager &DbManager::operator= (const DbManager &other)
{
	(void)other;
	assert (!"DbManager assigned");
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
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	migratorWorker.isCurrent (returner, monitor);
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
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	migratorWorker.isEmpty (returner, monitor);
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
			ThreadSafeInterface rootInterface (rootInfo);
			InterfaceWorker rootInterfaceWorker (rootInterface);

			doOpenInterface (rootInterfaceWorker, parent);

			Returner<void> returner;
			SignalOperationMonitor monitor;
			QObject::connect (&monitor, SIGNAL (canceled ()), &rootInterface, SLOT (cancelConnection ()), Qt::DirectConnection);

			std::cout << "+gall" << std::endl;
			rootInterfaceWorker.grantAll (returner, monitor, info.database, info.username, info.password);
			MonitorDialog::monitor (monitor, "Benutzer anlegen", parent);
			returner.wait ();
			std::cout << "-gall" << std::endl;

			rootInterface.close ();
		}
		catch (AccessDeniedException) // Actually: only 1045
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

	ThreadSafeInterface createInterface (createInfo);
	InterfaceWorker createInterfaceWorker (createInterface);

	doOpenInterface (createInterfaceWorker, parent);

	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &createInterface, SLOT (cancelConnection ()), Qt::DirectConnection);
	createInterfaceWorker.createDatabase (returner, monitor, info.database);
	MonitorDialog::monitor (monitor, "Datenbank anlegen", parent);
	returner.wait ();
}

void DbManager::createSampleLaunchMethods (QWidget *parent)
{
	QList<LaunchMethod> sampleLaunchMethods;

	LaunchMethod winchA;
	winchA.name="Winde Verein A";
	winchA.shortName="WA";
	winchA.logString="W";
	winchA.keyboardShortcut="A";
	winchA.type=LaunchMethod::typeWinch;
	winchA.personRequired=true;
	sampleLaunchMethods.append (winchA);

	LaunchMethod winchB;
	winchB.name="Winde Verein B";
	winchB.shortName="WB";
	winchB.logString="W";
	winchB.keyboardShortcut="B";
	winchB.type=LaunchMethod::typeWinch;
	winchB.personRequired=true;
	sampleLaunchMethods.append (winchB);

	LaunchMethod airtowEfgh;
	airtowEfgh.name="D-EFGH";
	airtowEfgh.shortName="GH";
	airtowEfgh.logString="F";
	airtowEfgh.keyboardShortcut="G";
	airtowEfgh.type=LaunchMethod::typeAirtow;
	airtowEfgh.towplaneRegistration="D-EFGH";
	airtowEfgh.personRequired=true;
	sampleLaunchMethods.append (airtowEfgh);

	LaunchMethod airtowMnop;
	airtowMnop.name="D-MNOP";
	airtowMnop.shortName="OP";
	airtowMnop.logString="F";
	airtowMnop.keyboardShortcut="O";
	airtowMnop.type=LaunchMethod::typeAirtow;
	airtowMnop.towplaneRegistration="D-MNOP";
	airtowMnop.personRequired=true;
	sampleLaunchMethods.append (airtowMnop);

	LaunchMethod airtowOther;
	airtowOther.name="F-Schlepp (sonstige)";
	airtowOther.shortName="FS";
	airtowOther.logString="F";
	airtowOther.keyboardShortcut="F";
	airtowOther.type=LaunchMethod::typeAirtow;
	airtowOther.personRequired=true;
	sampleLaunchMethods.append (airtowOther);

	LaunchMethod selfLaunch;
	selfLaunch.name="Eigenstart";
	selfLaunch.shortName="ES";
	selfLaunch.logString="E";
	selfLaunch.keyboardShortcut="E";
	selfLaunch.type=LaunchMethod::typeSelf;
	selfLaunch.personRequired=false;
	sampleLaunchMethods.append (selfLaunch);


	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.createObjects (returner, monitor, sampleLaunchMethods);
	MonitorDialog::monitor (monitor, "Beispielstartarten erstellen", parent);
	returner.wait ();
}


void DbManager::checkVersion (QWidget *parent)
{
	if (isEmpty (parent))
	{
		confirmOrCancel ("Datenbank leer",
			"Die Datenbank ist leer. Soll sie jetzt erstellt werden?", parent);

		Returner<void> returner;
		SignalOperationMonitor monitor;
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
		migratorWorker.loadSchema (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait (); // Required so any exceptions are rethrown

		// After loading the schema, the database must be current.
		// TODO different message if canceled
		ensureCurrent ("Datenbank ist nach Erstellen nicht aktuell.", parent);

		createSampleLaunchMethods (parent);
	}
	else if (!isCurrent (parent))
	{
		// TODO try to reuse monitor and dialog
		Returner<quint64> currentVersionReturner;
		SignalOperationMonitor currentVersionMonitor;
		QObject::connect (&currentVersionMonitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
		migratorWorker.currentVersion (currentVersionReturner, currentVersionMonitor);
		MonitorDialog::monitor (currentVersionMonitor, "Verbindungsaufbau", parent);
		quint64 currentVersion=currentVersionReturner.returnedValue ();

		quint64 latestVersion=Migrator::latestVersion ();

		Returner<QList<quint64> > pendingMigrationsReturner;
		SignalOperationMonitor pendingMigrationsMonitor;
		QObject::connect (&pendingMigrationsMonitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
		migratorWorker.pendingMigrations (pendingMigrationsReturner, pendingMigrationsMonitor);
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
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
		migratorWorker.migrate (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait (); // Required so any exceptions are rethrown

		// After migrating, the database must be current.
		// TODO different message if canceled
		ensureCurrent ("Datenbank ist nach der Aktualisierung nicht aktuell.", parent);
	}
}

void DbManager::doOpenInterface (InterfaceWorker &worker, QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &worker.getInterface (), SLOT (cancelConnection ()), Qt::DirectConnection);
	worker.open (returner, monitor);
	MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
	returner.wait ();
}

void DbManager::openInterface (QWidget *parent)
{
	// Open the interface. This would also be done automatically, but this
	// allows us to detect a missing database before checking the version.
	try
	{
		doOpenInterface (interfaceWorker, parent);
	}
	catch (DatabaseDoesNotExistException)
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
		doOpenInterface (interfaceWorker, parent);

		Returner<void> returner;
		SignalOperationMonitor monitor;
		QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
		migratorWorker.loadSchema (returner, monitor);
		MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
		returner.wait (); // Required so any exceptions are rethrown

		// After loading the schema, the database must be current.
		// TODO different message if canceled
		ensureCurrent ("Nach dem Laden ist die Datenbank nicht aktuell.", parent);

		createSampleLaunchMethods (parent);
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
	}
	catch (...)
	{
		// TODO check it works even if it's not open
		interface.close ();
		throw;
	}
}

bool DbManager::connect (QWidget *parent)
{
	try
	{
		try
		{
			connectImpl (parent);
			return true;
		}
		catch (AccessDeniedException)
		{
			grantPermissions (parent);
			connectImpl (parent);
			return true;
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
	catch (SqlException &ex)
	{
		QSqlError error=ex.error;

		QString text=QString (
			"Beim Verbindungsaufbau ist ein Fehler aufgetreten: %1"
			" (Fehlercode %2, Typ %3)"
			).arg (error.databaseText ()).arg (error.number ()).arg (error.type ());
		showWarning ("Fehler beim Verbindungsaufbau", text, parent);
	}

	return false;
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
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	cacheWorker.refreshAll (returner, monitor);
	MonitorDialog::monitor (monitor, "Daten abrufen", parent);
	returner.wait ();
}

void DbManager::fetchFlights (QDate date, QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	cacheWorker.fetchFlightsOther (returner, monitor, date);
	MonitorDialog::monitor (monitor, utf8 ("Flüge abrufen"), parent);
	returner.wait ();
}

template<class T> bool DbManager::objectUsed (dbId id, QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.objectUsed<T> (returner, monitor, id);
	MonitorDialog::monitor (monitor, utf8 ("%1 prüfen").arg (T::objectTypeDescription ()), parent);
	return returner.returnedValue ();
}

template<class T> void DbManager::deleteObject (dbId id, QWidget *parent)
{
	Returner<int> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.deleteObject<T> (returner, monitor, id);
	MonitorDialog::monitor (monitor, utf8 ("%1 löschen").arg (T::objectTypeDescription ()), parent);
	returner.wait ();
}

template<class T> bool DbManager::createObject (T &object, QWidget *parent)
{
	Returner<dbId> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.createObject (returner, monitor, object);
	MonitorDialog::monitor (monitor, utf8 ("%1 anlegen").arg (T::objectTypeDescription ()), parent);
	return idValid (returner.returnedValue ());
}

template<class T> int DbManager::updateObject (const T &object, QWidget *parent)
{
	Returner<int> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.updateObject (returner, monitor, object);
	MonitorDialog::monitor (monitor, utf8 ("%1 aktualisieren").arg (T::objectTypeDescription ()), parent);
	return returner.returnedValue ();
}



// ***************************
// ** Method instantiations **
// ***************************

#	define INSTANTIATE_TEMPLATES(T) \
		template bool DbManager::objectUsed  <T> (dbId id        , QWidget *parent); \
		template void DbManager::deleteObject<T> (dbId id        , QWidget *parent); \
		template bool DbManager::createObject<T> (T &object      , QWidget *parent); \
		template int  DbManager::updateObject<T> (const T &object, QWidget *parent); \
	// Empty line

INSTANTIATE_TEMPLATES (Person      )
INSTANTIATE_TEMPLATES (Plane       )
INSTANTIATE_TEMPLATES (Flight      )
INSTANTIATE_TEMPLATES (LaunchMethod)

#	undef INSTANTIATE_TEMPLATES
