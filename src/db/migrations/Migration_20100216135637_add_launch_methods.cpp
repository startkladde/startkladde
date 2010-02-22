#include "Migration_20100216135637_add_launch_methods.h"

#include <iostream>

#include "src/model/LaunchMethod.h"
#include "src/config/Options.h"
#include "src/util/qString.h"

Migration_20100216135637_add_launch_methods::Migration_20100216135637_add_launch_methods (DatabaseInterface &databaseInterface):
	Migration (databaseInterface)
{
}

Migration_20100216135637_add_launch_methods::~Migration_20100216135637_add_launch_methods ()
{
}

void Migration_20100216135637_add_launch_methods::up ()
{
	createTable  ("launch_methods"); // Creates the id column
	addColumn ("launch_methods", "name"                 , dataTypeString    ());
	addColumn ("launch_methods", "short_name"           , dataTypeString    ());
	addColumn ("launch_methods", "log_string"           , dataTypeString    ());
	addColumn ("launch_methods", "keyboard_shortcut"    , dataTypeCharacter ());
	// Use a string type because SQLite does not support enums
	addColumn ("launch_methods", "type"                 , dataTypeString    ());
	addColumn ("launch_methods", "towplane_registration", dataTypeString    ());
	addColumn ("launch_methods", "person_required"      , dataTypeBoolean   ());
	addColumn ("launch_methods", "comments"             , dataTypeString    ());

	try
	{
		transaction ();

		foreach (LaunchMethod launchMethod, opts.configuredLaunchMethods)
		{
			// Don't use the methods of Database or LaunchMethod - they use the
			// current schema, we need to use the schema after this migration.
			std::cout << "Importing launch method: " << launchMethod.toString () << std::endl;

			QSqlQuery query=prepareQuery (
				"INSERT INTO launch_methods"
				"(id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments)"
				"values (?,?,?,?,?,?,?,?,?)"
				);

			query.addBindValue (launchMethod.getId ());
			query.addBindValue (launchMethod.name);
			query.addBindValue (launchMethod.shortName);
			query.addBindValue (launchMethod.logString);
			query.addBindValue (launchMethod.keyboardShortcut);
			query.addBindValue (LaunchMethod::typeToDb (launchMethod.type));
			query.addBindValue (launchMethod.towplaneRegistration);
			query.addBindValue (launchMethod.personRequired);
			query.addBindValue (launchMethod.comments);

			executeQuery (query);
		}

		commit ();
	}
	catch (...)
	{
		rollback ();
		throw;
	}
}

void Migration_20100216135637_add_launch_methods::down ()
{
	dropTable ("launch_methods");
}

