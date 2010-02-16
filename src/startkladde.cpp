#include <unistd.h>

#include <QApplication>

#include "src/text.h"
#include "src/config/Options.h"
#include "src/gui/windows/MainWindow.h"
#include "src/gui/windows/SplashScreen.h"
#include "src/plugins/ShellPlugin.h"
#include "src/db/Database.h"
#include "src/db/migration/Migrator.h"
#include "src/db/migration/MigrationFactory.h"

// Testen des Wetterplugins
//#include "WeatherDialog.h"
//#include "ShellPlugin.h"

void display_help ()
	/*
	 * Displays a brief parameter information.
	 */
{
	std::cout << "usage: startkladde [options]" << std::endl;
	std::cout << "  options:" << std::endl;
	Options::display_options ("    ");
}

#include "src/model/Person.h"


void test_database (Database &db, int argc, char **argv)
{
	bool ok=db.open (opts.databaseInfo);

	if (!ok)
	{
		std::cout << "Database failed to open: " << db.lastError ().text () << std::endl;
		return;
	}

	if (argc<2)
	{
		std::cout << "up, down, migrate, version, list" << std::endl;
		return;
	}

	QString cmd (argv[1]);

	if (cmd=="up")
	{
		Migrator m (db);
		m.up ();
	}
	else if (cmd=="down")
	{
		Migrator m (db);
		m.down ();
	}
	else if (cmd=="migrate")
	{
		Migrator m (db);
		m.migrate ();
	}
	else if (cmd=="version")
	{
		Migrator m (db);
		std::cout << "Version is " << m.currentVersion () << std::endl;
	}
	else if (cmd=="list")
	{
		MigrationFactory factory;
		foreach (quint64 version, factory.availableVersions ())
			std::cout << version << " - " << factory.migrationName (version) << std::endl;

	}
	else if (cmd!="test")
	{
		std::cout << "Unrecognized" << std::endl;
	}

	if (cmd!="test") return;

	std::cout << std::endl;
	std::cout << "Get people" << std::endl;
	QList<Person> people=db.getObjects<Person> ();
    foreach (const Person &person, people)
    	std::cout << person.toString ().toUtf8 () << std::endl;

	std::cout << std::endl;
	std::cout << "Get planes" << std::endl;
	QList<Plane> planes=db.getObjects<Plane> ();
    foreach (const Plane &plane, planes)
    	std::cout << plane.toString ().toUtf8 () << std::endl;

	std::cout << std::endl;
	std::cout << "Get flights" << std::endl;
	QList<Flight> flights=db.getObjects<Flight> ();
    foreach (const Flight &flight, flights)
    	std::cout << flight.toString ().toUtf8 () << std::endl;

	std::cout << std::endl;
	std::cout << "Get prepared flights" << std::endl;
	flights=db.getPreparedFlights ();
    foreach (const Flight &flight, flights)
    	std::cout << flight.toString ().toUtf8 () << std::endl;

	std::cout << std::endl;
	std::cout << "Get flights of 2010-01-23" << std::endl;
	flights=db.getFlightsDate (QDate (2010, 1, 23));
    foreach (const Flight &flight, flights)
    	std::cout << flight.toString ().toUtf8 () << std::endl;

	std::cout << std::endl;
	std::cout << "Get launch types" << std::endl;
	QList<LaunchType> launchTypes=db.getObjects<LaunchType> ();
    foreach (const LaunchType &launchType, launchTypes)
    	std::cout << launchType.toString ().toUtf8 () << std::endl;

    std::cout << std::endl;
	std::cout << "List airfields" << std::endl;
	std::cout << db.listAirfields ().join (", ") << std::endl;

    std::cout << std::endl;
	std::cout << "List accounting notes" << std::endl;
	std::cout << db.listAccountingNotes ().join (", ") << std::endl;

    std::cout << std::endl;
	std::cout << "List clubs" << std::endl;
	std::cout << db.listClubs ().join (", ") << std::endl;

    std::cout << std::endl;
	std::cout << "List plane types" << std::endl;
	std::cout << db.listPlaneTypes ().join (", ") << std::endl;


}

int main (int argc, char **argv)
	/*
	 * Starts the startkladde program.
	 * Parameters:
	 *   - argc, argv: the usual. Some of the Options can be passed here.
	 * Return value:
	 *   the return value of the QApplication, and thus of the main window.
	 */
{
	try
	{
		Database testDb;
		opts.parse_arguments (argc, argv);
		opts.read_config_files (&testDb, NULL, argc, argv);
		test_database (testDb, argc, argv);
	}
	catch (Database::QueryFailedException &ex)
	{
		std::cout << "QueryFailedException" << std::endl;
		std::cout << "  Query: " << ex.query.lastQuery () << std::endl;
		std::cout << "  Error: " << ex.query.lastError ().text () << std::endl;
	}
	return 0;

	// DbEvents are used as parameters for signals emitted by tasks running on
	// a background thread. These connections must be queued, so the parameter
	// types must be registered.
	qRegisterMetaType<DbEvent> ("DbEvent");
	qRegisterMetaType<DataStorage::State> ("DataStorage::State");

	Database db;
	QList<ShellPlugin *> plugins;

	if (opts.need_display ())
		opts.do_display ();
	else if (opts.display_help)
		display_help ();
	else
	{
		opts.read_config_files (&db, &plugins, argc, argv);
		//QApplication::setDesktopSettingsAware (FALSE); // I know better than the user
		QApplication a (argc, argv);

		// Put light.{la,so} to styles/
		//a.setStyle ("light, 3rd revision");
		if (!opts.style.isEmpty ()) a.setStyle (opts.style);

//		db.display_queries=opts.display_queries;

		MainWindow w (NULL, &db, plugins);

		// Let the plugins initialize
		sched_yield ();



		w.showMaximized ();
//		w.show ();
		int ret=a.exec();

		foreach (ShellPlugin *plugin, plugins)
		{
//			std::cout << "Terminating plugin " << plugin->get_caption () << std::endl;
			plugin->terminate ();
			sched_yield ();
		}

		return ret;
	}
}

