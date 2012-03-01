/*
 * Improvements:
 *   - have the whole creation procress run in the background. This would
 *     remove a lot of monitor blocks (reducing code duplication) and interface
 *     worker (thread) creations and make it
 *     easier to leave the dialog open and provide progress feedback. But this
 *     requires getting user input from a background thread.
 */
#include "DbManager.h"

#include <iostream> // remove

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
#include "src/config/Settings.h"

#include "src/concurrent/DefaultQThread.h" //remove

DbManager::DbManager (const DatabaseInfo &info):
	state (stateDisconnected),
	interface (info, 5000, 2000), db (interface), cache (db),
	interfaceWorker (interface), dbWorker (db), migratorWorker (interface), cacheWorker (cache)
{
	QObject::connect (&interface, SIGNAL (readTimeout ()), this, SIGNAL (readTimeout ()));
	QObject::connect (&interface, SIGNAL (readResumed ()), this, SIGNAL (readResumed ()));
	QObject::connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));

	QObject::connect (&migratorWorker, SIGNAL (migrationStarted ()), this, SIGNAL (migrationStarted ()));
	QObject::connect (&migratorWorker, SIGNAL (migrationEnded   ()), this, SIGNAL (migrationEnded   ()));
}

DbManager::DbManager (const DbManager &other):
	QObject (),
	interface (other.interface.getInfo ()), db (interface), cache (db),
	interfaceWorker (interface), dbWorker (db), migratorWorker (interface), cacheWorker (cache)
{
	assert (!notr ("DbManager copied"));
}

DbManager &DbManager::operator= (const DbManager &other)
{
	(void)other;
	assert (!notr ("DbManager assigned"));
}

DbManager::~DbManager ()
{
}

void DbManager::setState (State newState)
{
	state=newState;
	emit stateChanged (state);
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
	MonitorDialog::monitor (monitor, tr ("Connecting"), parent); //tr Verbindungsaufbau
	return returner.returnedValue ();
}

void DbManager::ensureCurrent (const QString &message, QWidget *parent)
{
	if (!isCurrent (parent))
		throw ConnectFailedException (message);
}

//bool DbManager::isEmpty (QWidget *parent)
//{
//	Returner<bool> returner;
//	SignalOperationMonitor monitor;
//	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
//	migratorWorker.isEmpty (returner, monitor);
//	MonitorDialog::monitor (monitor, "Verbindungsaufbau", parent);
//	return returner.returnedValue ();
//}

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
	QString title=tr ("Database password required"); //tr Datenbank-Passwort benötigt
	QString text=tr (
		"The database user %1 does not exist, the given password\n"
		"is not correct or the user has insufficient access to\n"
		"the database %2. To correct this automatically, the password\n"
		"of the database user root is required (this password can be\n"
		"different from the one of the system user root).\n"
		"Please enter the password for the database user root:")
//tr		"Der Datenbankbenutzer %1 existiert nicht, das angegebene Passwort\n"
//tr		"stimmt nicht oder der Benutzer hat unzureichende Zugriffsrechte auf\n"
//tr		"die Datenbank %2. Zur automatischen Korrektur wird das Passwort des\n"
//tr		"Datenbankbenutzers root benötigt (dieses Passwort kann ein anderes\n" /*utf8*/
//tr		"als das für den Systembenutzer root sein)\n" /*utf8*/
//tr		"Bitte das Kennwort für den Datenbankbenutzer root eingeben:") /*utf8*/
		.arg (info.username, info.database);

	bool retry=true;
	while (retry)
	{
		retry=false;

		bool ok;
		QString rootPassword=QInputDialog::getText (parent,
			title, text, QLineEdit::Password, notr (""), &ok);

		if (!ok)
			throw DbManager::ConnectCanceledException ();

		DatabaseInfo rootInfo=info;
		rootInfo.database=notr ("");
		rootInfo.username=notr ("root");
		rootInfo.password=rootPassword;

		try
		{
			ThreadSafeInterface rootInterface (rootInfo);
			InterfaceWorker rootInterfaceWorker (rootInterface);

			doOpenInterface (rootInterfaceWorker, parent);

			Returner<void> returner;
			SignalOperationMonitor monitor;
			QObject::connect (&monitor, SIGNAL (canceled ()), &rootInterface, SLOT (cancelConnection ()), Qt::DirectConnection);

			rootInterfaceWorker.grantAll (returner, monitor, info.database, info.username, info.password);
			MonitorDialog::monitor (monitor, tr ("Creating user"), parent); //tr Benutzer anlegen
			returner.wait ();

			rootInterface.close ();
		}
		catch (AccessDeniedException) // Actually: only 1045
		{
			text=tr (
				"Logging in as root failed. The given password\n"
				"may not be correct.\n"
				"Please enter the password for the database user root:");
//				"Anmeldung als root verweigert. Möglicherweise ist das\n" /*utf8*/
//				"angegebene Kennwort nicht richtig.\n"
//				"Bitte das Kennwort für root eingeben:"); /*utf8*/
			retry=true;
		}
	}
}

void DbManager::createDatabase (QWidget *parent)
{
	const DatabaseInfo &info=interface.getInfo ();

	DatabaseInfo createInfo=info;
	createInfo.database=notr ("");

	ThreadSafeInterface createInterface (createInfo);
	InterfaceWorker createInterfaceWorker (createInterface);

	doOpenInterface (createInterfaceWorker, parent);

	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &createInterface, SLOT (cancelConnection ()), Qt::DirectConnection);
	createInterfaceWorker.createDatabase (returner, monitor, info.database);
	MonitorDialog::monitor (monitor, tr ("Creating database"), parent); //tr Datenbank anlegen
	returner.wait ();
}

void DbManager::createSampleLaunchMethods (QWidget *parent)
{
	QList<LaunchMethod> sampleLaunchMethods;

	LaunchMethod winchA;
	winchA.name=tr ("Winch club A"); //tr Winde Verein A
	winchA.shortName=tr ("WA", "Winch club A"); //tr WA
	winchA.logString=tr ("W", "Winch logbook label"); //tr W
	winchA.keyboardShortcut=tr ("A", "Winch club A keyboard shortcut"); //tr A
	winchA.type=LaunchMethod::typeWinch;
	winchA.personRequired=true;
	sampleLaunchMethods.append (winchA);

	LaunchMethod winchB;
	winchB.name=tr ("Winch club B"); //tr Winde Verein B
	winchB.shortName=tr ("WB", "Winch club B"); //tr WB
	winchB.logString=tr ("W", "Winch logbook label"); //tr W
	winchB.keyboardShortcut=tr ("B", "Winch club B keyboard shortcut"); //tr B
	winchB.type=LaunchMethod::typeWinch;
	winchB.personRequired=true;
	sampleLaunchMethods.append (winchB);

	LaunchMethod airtowEfgh;
	airtowEfgh.name=tr ("TODO"); //tr D-EFGH
	airtowEfgh.shortName=tr ("TODO"); //tr GH
	airtowEfgh.logString=tr ("TODO"); //tr F
	airtowEfgh.keyboardShortcut=tr ("TODO"); //tr G
	airtowEfgh.type=LaunchMethod::typeAirtow;
	airtowEfgh.towplaneRegistration=tr ("TODO"); //tr D-EFGH
	airtowEfgh.personRequired=true;
	sampleLaunchMethods.append (airtowEfgh);

	LaunchMethod airtowMnop;
	airtowMnop.name=tr ("TODO"); //tr D-MNOP
	airtowMnop.shortName=tr ("TODO"); //tr OP
	airtowMnop.logString=tr ("TODO"); //tr F
	airtowMnop.keyboardShortcut=tr ("TODO"); //tr O
	airtowMnop.type=LaunchMethod::typeAirtow;
	airtowMnop.towplaneRegistration=tr ("TODO"); //tr D-MNOP
	airtowMnop.personRequired=true;
	sampleLaunchMethods.append (airtowMnop);

	LaunchMethod airtowOther;
	airtowOther.name=tr ("TODO"); //tr F-Schlepp (sonstige)
	airtowOther.shortName=tr ("TODO"); //tr FS
	airtowOther.logString=tr ("TODO"); //tr F
	airtowOther.keyboardShortcut=tr ("TODO"); //tr F
	airtowOther.type=LaunchMethod::typeAirtow;
	airtowOther.personRequired=true;
	sampleLaunchMethods.append (airtowOther);

	LaunchMethod selfLaunch;
	selfLaunch.name=tr ("TODO"); //tr Eigenstart
	selfLaunch.shortName=tr ("TODO"); //tr ES
	selfLaunch.logString=tr ("TODO"); //tr E
	selfLaunch.keyboardShortcut=tr ("TODO"); //tr E
	selfLaunch.type=LaunchMethod::typeSelf;
	selfLaunch.personRequired=false;
	sampleLaunchMethods.append (selfLaunch);


	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.createObjects (returner, monitor, sampleLaunchMethods);
	MonitorDialog::monitor (monitor, tr ("Creating example launch methods"), parent); //tr Beispielstartarten erstellen
	returner.wait ();
}


void DbManager::checkVersion (QWidget *parent)
{
	quint64 currentVersion;
	int numPendingMigrations;

	Returner<Migrator::Action> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	migratorWorker.getRequiredAction (returner, monitor, &currentVersion, &numPendingMigrations);
	MonitorDialog::monitor (monitor, tr ("Connecting"), parent); //tr Verbindungsaufbau
	Migrator::Action action=returner.returnedValue ();

	switch (action)
	{
		case Migrator::actionLoad:
		{
			confirmOrCancel (tr ("Database empty"), //tr Datenbank leer
				tr ("TODO").arg (interface.getInfo ().toString ()), parent); //tr Die Datenbank %1 ist leer oder unvollständig. Soll sie jetzt erstellt werden?

			Returner<void> returner;
			SignalOperationMonitor monitor;
			QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
			migratorWorker.loadSchema (returner, monitor);
			MonitorDialog::monitor (monitor, tr ("Connecting"), parent); //tr Verbindungsaufbau
			returner.wait (); // Required so any exceptions are rethrown

			// After loading the schema, the database must be current.
			ensureCurrent (tr ("Database not current after creating"), parent); //tr Datenbank ist nach Erstellen nicht aktuell.

			createSampleLaunchMethods (parent);
		} break;
		case Migrator::actionMigrate:
		{
			quint64 latestVersion=Migrator::latestVersion ();

			confirmOrCancel (tr ("Database not current"), tr ( //tr Datenbank nicht aktuell
					"The database is not up to date:\n"
					"  - Current version: %1\n"
					"  - Up-to-date version: %2\n"
					"  - Number of missing migrations: %3\n"
					"\n"
					"Attention: the update should not be interrupted because this can lead to an inconsistent database which cannot be repaired automatically.\n"
					"Before updating, a backup of the database should be made.\n"
					"\n"
					"Update the database now?"
//					"Die Datenbank ist nicht aktuell:\n"
//					"  - Momentane Version: %1\n"
//					"  - Aktuelle Version: %2\n"
//					"  - Anzahl ausstehender Migrationen: %3\n"
//					"\n"
//					"Achtung: die Aktualisierung sollte nicht unterbrochen werden, da dies zu einer inkonsistenten Datenbank führen kann, die nicht automatisch repariert werden kann.\n" /*utf8*/
//					"Vor dem Aktualisieren der Datenbank sollte eine Sicherungskopie der Datenbank erstellt werden.\n"
//					"\n"
//					"Soll die Datenbank jetzt aktualisiert werden?"
				).arg (currentVersion).arg (latestVersion).arg (numPendingMigrations), parent);

			Returner<void> returner;
			SignalOperationMonitor monitor;
			QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
			migratorWorker.migrate (returner, monitor);
			MonitorDialog::monitor (monitor, tr ("Connecting"), parent); //tr Verbindungsaufbau
			returner.wait (); // Required so any exceptions are rethrown

			// After migrating, the database must be current.
			ensureCurrent (tr ("The database is not up to date after updating."), parent); //tr Datenbank ist nach der Aktualisierung nicht aktuell.
		} break;
		case Migrator::actionNone:
			// Nothing to do, the database is current
			break;
		// no default
	}
}

void DbManager::doOpenInterface (InterfaceWorker &worker, QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &worker.getInterface (), SLOT (cancelConnection ()), Qt::DirectConnection);
	worker.open (returner, monitor);
	MonitorDialog::monitor (monitor, tr ("Connecting"), parent); //tr Verbindungsaufbau
	returner.wait ();

	// Now that the interface is open, we can start the keepalive
	worker.getInterface ().setKeepaliveEnabled (true);
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

		confirmOrCancel (tr ("Create database?"), QString ( //tr Datenbank erstellen?
			tr ("TODO")) //tr Die Datenbank %1 existiert nicht. Soll sie erstellt werden?
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
		MonitorDialog::monitor (monitor, tr ("Connecting"), parent); //tr Verbindungsaufbau
		returner.wait (); // Required so any exceptions are rethrown

		// After loading the schema, the database must be current.
		ensureCurrent (tr ("After loading, the database is not up to date."), parent); //tr Nach dem Laden ist die Datenbank nicht aktuell.

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
	setState (stateConnecting);

	try
	{
		try
		{
			connectImpl (parent);
			setState (stateConnected);
			return true;
		}
		catch (AccessDeniedException)
		{
			grantPermissions (parent);
			connectImpl (parent);
			setState (stateConnected);
			return true;
		}
		// TODO also for access denied during query (1142)
	}
	catch (DbManager::ConnectCanceledException)
	{
		showWarning (tr ("Connection canceled"), //tr Verbindungsaufbau abgebrochen
			tr ("TODO"), //tr Der Verbindungsaufbau wurde abgebrochen
			parent);
	}
	catch (OperationCanceledException &ex)
	{
		showWarning (tr ("Connection canceled"), //tr Verbindungsaufbau abgebrochen
			tr ("TODO"), //tr Der Verbindungsaufbau wurde abgebrochen
			parent);
	}
	catch (ConnectFailedException &ex)
	{
		showWarning (tr ("Connection failed"), //tr Verbindungsaufbau fehlgeschlagen
			tr ("TODO").arg (ex.message), //tr Beim Verbindungsaufbau ist ein Fehler aufgetreten: %1
			parent);
	}
	catch (SqlException &ex)
	{
		QSqlError error=ex.error;

		QString text=tr (
			"Beim Verbindungsaufbau ist ein Fehler aufgetreten: %1"
			" (Fehlercode %2, Typ %3)"
			).arg (error.databaseText ()).arg (error.number ()).arg (error.type ());
		showWarning (tr ("Error while connecting"), text, parent); //tr Fehler beim Verbindungsaufbau
	}

	setState (stateDisconnected);
	return false;
}

void DbManager::disconnect ()
{
	interface.close ();
	cache.clear ();
	setState (stateDisconnected);
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
	MonitorDialog::monitor (monitor, tr ("Retrieving data"), parent); //tr Daten abrufen
	returner.wait ();
}

/**
 * Fetches the flights of a single date to the cache
 *
 * @param date
 * @param parent
 */
void DbManager::fetchFlights (QDate date, QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	cacheWorker.fetchFlightsOther (returner, monitor, date);
	MonitorDialog::monitor (monitor, tr ("Retrieving flights"), parent); //tr Flüge abrufen
	returner.wait ();
}

/**
 * Gets and returns the flights of a date range
 *
 * @param first first date of the range
 * @param last last date of the range
 * @param parent parent widget for progress dialog
 * @return the list of flights
 */
// FIXME throws?
// FIXME it might be better to implement a template getObjects method and do the
// query selection (and potentially the after filter) outside of this method
QList<Flight> DbManager::getFlights (const QDate &first, const QDate &last, QWidget *parent)
{
	Returner<QList<Flight> > returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	// FIXME make sure that not dbWorker method is called with a temporary as
	// a reference
	Query condition=Flight::dateRangeSupersetCondition (first, last);
	dbWorker.getObjects<Flight> (returner, monitor, condition);
	MonitorDialog::monitor (monitor, tr ("Retrieving flights"), parent); //tr Flüge abrufen
	QList<Flight> candidates=returner.returnedValue ();
	return Flight::dateRangeSupersetFilter (candidates, first, last);
	return returner.returnedValue ();
}

template<class T> void DbManager::refreshObjects (QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	cacheWorker.refreshObjects<T> (returner, monitor);
	MonitorDialog::monitor (monitor, tr ("Refreshing data"), parent); //tr Daten aktualisieren
	returner.wait ();
}

template<class T> bool DbManager::objectUsed (dbId id, QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.objectUsed<T> (returner, monitor, id);
	MonitorDialog::monitor (monitor, tr ("Checking %1").arg (T::objectTypeDescription ()), parent); //tr %1 prüfen
	return returner.returnedValue ();
}

// Improvement: atomic used check and delete
template<class T> void DbManager::deleteObject (dbId id, QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.deleteObject<T> (returner, monitor, id);
	MonitorDialog::monitor (monitor, tr ("Deleting %1").arg (T::objectTypeDescription ()), parent); //tr %1 löschen
	returner.wait ();
}

// Improvement: atomic used check and delete
template<class T> void DbManager::deleteObjects (const QList<dbId> &ids, QWidget *parent)
{
	Returner<int> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.deleteObjects<T> (returner, monitor, ids);
	MonitorDialog::monitor (monitor, tr ("Deleting %1").arg (T::objectTypeDescriptionPlural ()), parent); //tr %1 löschen
	returner.wait ();
}

template<class T> dbId DbManager::createObject (T &object, QWidget *parent)
{
	Returner<dbId> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.createObject (returner, monitor, object);
	MonitorDialog::monitor (monitor, tr ("Creating %1").arg (T::objectTypeDescription ()), parent); //tr %1 anlegen
	return returner.returnedValue ();
}

template<class T> int DbManager::updateObject (const T &object, QWidget *parent)
{
	Returner<bool> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	dbWorker.updateObject (returner, monitor, object);
	MonitorDialog::monitor (monitor, tr ("Updating %1").arg (T::objectTypeDescription ()), parent); //tr %1 aktualisieren
	return returner.returnedValue ();
}

void DbManager::executeQuery (const Query &query, const QString &statusText, QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	interfaceWorker.executeQuery (returner, monitor, query);
	MonitorDialog::monitor (monitor, statusText, parent);
	returner.wait ();
}

void DbManager::transaction (QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	interfaceWorker.transaction (returner, monitor);
	MonitorDialog::monitor (monitor, tr ("Beginning transaction"), parent); //tr Transaktion beginnen
	returner.wait ();
}

void DbManager::commit (QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	interfaceWorker.commit (returner, monitor);
	MonitorDialog::monitor (monitor, tr ("Committing transaction"), parent); //tr Transaktion ausführen
	returner.wait ();
}

void DbManager::rollback (QWidget *parent)
{
	Returner<void> returner;
	SignalOperationMonitor monitor;
	QObject::connect (&monitor, SIGNAL (canceled ()), &interface, SLOT (cancelConnection ()), Qt::DirectConnection);
	interfaceWorker.rollback (returner, monitor);
	MonitorDialog::monitor (monitor, tr ("Rolling back transaction"), parent); //tr Transaktion abbrechen
	returner.wait ();
}


// **********************
// ** Database updates **
// **********************

QString DbManager::mergeDeleteWarningTitle (int notDeletedCount, int deletedCount)
{
	(void)deletedCount;

	if (notDeletedCount>1)
		return tr ("People still in use"); //tr Personen noch in Benutzung
	else
		return tr ("Person still in use"); //tr Person noch in Benutzung
}

QString DbManager::mergeDeleteWarningText (int notDeletedCount, int deletedCount)
{
	QString text;

	if (notDeletedCount>1)
		text=tr ("TODO").arg (notDeletedCount); //tr Nach dem Zusammenfassen sind noch %1 Personen in Benutzung.
	else
		text=tr ("TODO"); //tr Nach dem Zusammenfassen ist noch eine Person in Benutzung.

	if (deletedCount==0)
		text+=tr ("TODO"); //tr  Es wird keine Person gelöscht.
	else if (deletedCount==1)
		text+=tr ("TODO"); //tr  Es wird nur eine Person gelöscht.
	else
		text+=tr ("TODO").arg (deletedCount); //tr  Es werden nur %1 Personen gelöscht.

	return text;
}



// TODO document bad coding practice: friend class for db.emitDbEvent,
// knowledge about Person in DbManager, but doing it right is complex
// TODO: replace with a worker class, and uninclude Person.h from header
/**
 * This method will not throw OperationCanceledException
 *
 * @param correctPerson
 * @param wrongPeople
 * @param parent
 */
void DbManager::mergePeople (const Person &correctPerson, const QList<Person> &wrongPeople, QWidget *parent)
{
	// Determine the ID of the correct person
	dbId correctId=correctPerson.getId ();

	// Determine the IDs of the wrong people
	QList<QVariant> wrongIds;
	foreach (const Person &person, wrongPeople)
		wrongIds.append (person.getId ());

	try
	{
		// Execute the queries
		// TODO single progress indicator
		transaction (parent);
		executeQuery (Query::updateColumnValue (notr ("flights"), notr ("pilot_id")   , correctId, wrongIds), tr ("TODO")       , parent); //tr Flüge: Piloteneinträge aktualisieren
		executeQuery (Query::updateColumnValue (notr ("flights"), notr ("copilot_id") , correctId, wrongIds), tr ("TODO")     , parent); //tr Flüge: Copiloteneinträge aktualisieren
		executeQuery (Query::updateColumnValue (notr ("flights"), notr ("towpilot_id"), correctId, wrongIds), tr ("TODO"), parent); //tr Flüge: Schlepppiloteneinträge aktualisieren
		executeQuery (Query::updateColumnValue (notr ("users")  , notr ("person_id")  , correctId, wrongIds), tr ("TODO") , parent); //tr Benutzer: Personenreferenzen aktualisieren
		commit (parent);

		// Emit the corresponding events

		// Make the Database emit a dbEvent for each affected flight (and user, if
		// we had a User class) so the cache and GUI will be updated
		foreach (Flight flight, cache.getAllKnownFlights ().getList ())
		{
			bool flightChanged=false;

			if (wrongIds.contains (flight.getPilotId    ())) { flight.setPilotId    (correctId); flightChanged=true; }
			if (wrongIds.contains (flight.getCopilotId  ())) { flight.setCopilotId  (correctId); flightChanged=true; }
			if (wrongIds.contains (flight.getTowpilotId ())) { flight.setTowpilotId (correctId); flightChanged=true; }

			if (flightChanged)
				db.emitDbEvent (DbEvent::changed<Flight> (flight));
		}
		// Users are not handled because we don't have users
	}
	catch (OperationCanceledException)
	{
		// TODO the cache may now be inconsistent
		// Don't emit the change events in this case.
		return;
	}


	// None of the wrong people should be in use any more. However, in case of
	// a bug in this method, a person may still be in use. Under no
	// circumstances may we delete a person that is still in use.

	// Make a list of IDs of wrong people we can delete
	QList<dbId> idsToDelete;
	try
	{
		foreach (const Person &person, wrongPeople)
			if (!objectUsed<Person> (person.getId (), parent))
				idsToDelete.append (person.getId ());
	}
	catch (OperationCanceledException)
	{
		// Data updated, but canceled before deletion - this is acceptable
		return;
	}

	// If the list of IDs to delete has a different size than the list of wrong
	// people, some of them were still in use.
	if (idsToDelete.size ()!=wrongPeople.size ())
	{
		int deletedCount=idsToDelete.size ();
		int notDeletedCount=wrongPeople.size ()-deletedCount;

		showWarning (
			mergeDeleteWarningTitle (notDeletedCount, deletedCount),
			mergeDeleteWarningText  (notDeletedCount, deletedCount),
			parent);
	}

	try
	{
		deleteObjects<Person> (idsToDelete, parent);
	}
	catch (OperationCanceledException &ex)
	{
		// TODO the cache may now be inconsistent
	}
}


// **************
// ** Settings **
// **************

void DbManager::settingsChanged ()
{
	interface.setInfo (Settings::instance ().databaseInfo);
}

// ***************************
// ** Method instantiations **
// ***************************

#	define INSTANTIATE_TEMPLATES(T) \
		template bool DbManager::objectUsed  <T> (dbId id        , QWidget *parent); \
		template void DbManager::deleteObject<T> (dbId id        , QWidget *parent); \
		template dbId DbManager::createObject<T> (T &object      , QWidget *parent); \
		template int  DbManager::updateObject<T> (const T &object, QWidget *parent); \
		template void DbManager::refreshObjects<T> (QWidget *parent);
		// Empty line

INSTANTIATE_TEMPLATES (Person      )
INSTANTIATE_TEMPLATES (Plane       )
INSTANTIATE_TEMPLATES (Flight      )
INSTANTIATE_TEMPLATES (LaunchMethod)

#	undef INSTANTIATE_TEMPLATES
