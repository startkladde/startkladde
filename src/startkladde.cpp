#include <unistd.h>

#include <iostream>

#include <QApplication>

#include "src/config/Options.h"
#include "src/gui/windows/MainWindow.h"
#include "src/plugins/ShellPlugin.h"
#include "src/db/Database.h"
#include "src/db/migration/Migrator.h"
#include "src/db/migration/MigrationFactory.h"
#include "src/db/schema/SchemaDumper.h"
#include "src/util/qString.h"

// For test_database
//#include "src/model/Plane.h"
//#include "src/model/Flight.h"
//#include "src/model/LaunchMethod.h"
//#include "src/model/Person.h"


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


//int test_database (Database &db)
//{
//
//	std::cout << std::endl;
//	std::cout << "Get people" << std::endl;
//	QList<Person> people=db.getObjects<Person> ();
//    foreach (const Person &person, people)
//    	std::cout << person.toString ().toUtf8 () << std::endl;
//
//	std::cout << std::endl;
//	std::cout << "Get planes" << std::endl;
//	QList<Plane> planes=db.getObjects<Plane> ();
//    foreach (const Plane &plane, planes)
//    	std::cout << plane.toString ().toUtf8 () << std::endl;
//
//	std::cout << std::endl;
//	std::cout << "Get flights" << std::endl;
//	QList<Flight> flights=db.getObjects<Flight> ();
//    foreach (const Flight &flight, flights)
//    	std::cout << flight.toString ().toUtf8 () << std::endl;
//
//	std::cout << std::endl;
//	std::cout << "Get prepared flights" << std::endl;
//	flights=db.getPreparedFlights ();
//    foreach (const Flight &flight, flights)
//    	std::cout << flight.toString ().toUtf8 () << std::endl;
//
//	std::cout << std::endl;
//	std::cout << "Get flights of 2010-01-23" << std::endl;
//	flights=db.getFlightsDate (QDate (2010, 1, 23));
//    foreach (const Flight &flight, flights)
//    	std::cout << flight.toString ().toUtf8 () << std::endl;
//
//	std::cout << std::endl;
//	std::cout << "Get launch methods" << std::endl;
//	QList<LaunchMethod> launchMethods=db.getObjects<LaunchMethod> ();
//    foreach (const LaunchMethod &launchMethod, launchMethods)
//    	std::cout << launchMethod.toString ().toUtf8 () << std::endl;
//
//    std::cout << std::endl;
//	std::cout << "List airfields" << std::endl;
//	std::cout << db.listAirfields ().join (", ") << std::endl;
//
//    std::cout << std::endl;
//	std::cout << "List accounting notes" << std::endl;
//	std::cout << db.listAccountingNotes ().join (", ") << std::endl;
//
//    std::cout << std::endl;
//	std::cout << "List clubs" << std::endl;
//	std::cout << db.listClubs ().join (", ") << std::endl;
//
//    std::cout << std::endl;
//	std::cout << "List plane types" << std::endl;
//	std::cout << db.listPlaneTypes ().join (", ") << std::endl;
//
//	return 0;
//}

int showGui (QApplication &a, Database &db, QList<ShellPlugin *> &plugins)
{
	//QApplication::setDesktopSettingsAware (FALSE); // I know better than the user

	// Put light.{la,so} to styles/
	//a.setStyle ("light, 3rd revision");
	if (!opts.style.isEmpty ()) a.setStyle (opts.style);
//		db.display_queries=opts.display_queries;

	MainWindow w (NULL, &db, plugins);

	// Let the plugins initialize
	sched_yield ();

	w.showMaximized ();
	int ret=a.exec();

	foreach (ShellPlugin *plugin, plugins)
	{
		//std::cout << "Terminating plugin " << plugin->get_caption () << std::endl;
		plugin->terminate ();
		sched_yield ();
	}

	return ret;
}

int doStuff (Database &db)
{

	// Tests ahead
	bool ok=db.open (opts.databaseInfo);

	if (!ok)
	{
		std::cout << "Database failed to open: " << db.lastError ().text () << std::endl;
		return 1;
	}

	if (opts.non_options[0]=="db:up")
		Migrator (db).up ();
	else if (opts.non_options[0]=="db:down")
		Migrator (db).down ();
	else if (opts.non_options[0]=="db:migrate")
		Migrator (db).migrate ();
	else if (opts.non_options[0]=="db:version")
		std::cout << "Version is " << Migrator (db).currentVersion () << std::endl;
	else if (opts.non_options[0]=="db:load")
		Migrator (db).loadSchema ();
	else if (opts.non_options[0]=="db:drop")
		Migrator (db).drop ();
	else if (opts.non_options[0]=="db:clear")
		Migrator (db).clear ();
	else if (opts.non_options[0]=="db:create")
		Migrator (db).create ();
//	else if (opts.non_options[0]=="db:test")
//		test_database (db);
	else if (opts.non_options[0]=="db:fail")
		db.executeQuery ("bam!");
	else if (opts.non_options[0]=="db:migrations")
	{
		MigrationFactory factory;
		foreach (quint64 version, factory.availableVersions ())
			std::cout << version << " - " << factory.migrationName (version) << std::endl;
	}
	else if (opts.non_options[0]=="db:dump")
	{
		Migrator m (db);
		if (!m.isCurrent ())
		{
			// TODO require --force switch if not current
			std::cerr << "Warning: the database is not current"  << std::endl;
		}

		SchemaDumper d (db);

		if (opts.non_options.size ()>1)
		{
			QString filename=opts.non_options[1];
			std::cout << QString ("Dumping schema to %1").arg (filename) << std::endl;
			d.dumpSchemaToFile (filename);
		}
		else
		{
			std::cout << d.dumpSchema () << std::endl;
		}
	}
	else
	{
		std::cout << "Unrecognized" << std::endl;
		return 1;
	}

	return 0;
}

int main (int argc, char **argv)
{
	// DbEvents are used as parameters for signals emitted by tasks running on
	// a background thread. These connections must be queued, so the parameter
	// types must be registered.
	qRegisterMetaType<DbEvent> ("DbEvent");
	qRegisterMetaType<DataStorage::State> ("DataStorage::State");

	int ret=0;

	try
	{
		if (opts.need_display ())
			opts.do_display ();
		else if (opts.display_help)
			display_help ();
		else
		{
			Database db;
			QList<ShellPlugin *> plugins;

			opts.read_config_files (&db, &plugins, argc, argv);
			QApplication a (argc, argv); // Always

			if (opts.non_options.empty ())
				ret=showGui (a, db, plugins);
			else
				ret=doStuff (db);
		}
	}
	catch (Database::QueryFailedException &ex)
	{
		std::cout << "QueryFailedException" << std::endl;
		std::cout << "  Query: " << ex.query.lastQuery () << std::endl;
		std::cout << "  Error: " << ex.query.lastError ().text () << std::endl;
		return 1;
	}

	return ret;
}
