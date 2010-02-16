#include "Migration_20100216135637_add_launch_methods.h"

#include "src/db/Database.h"

#include "src/model/LaunchMethod.h"
#include "src/config/Options.h"

Migration_20100216135637_add_launch_methods::Migration_20100216135637_add_launch_methods (Database &database):
	Migration (database)
{
}

Migration_20100216135637_add_launch_methods::~Migration_20100216135637_add_launch_methods ()
{
}

void Migration_20100216135637_add_launch_methods::up ()
{
	database.createTable  ("launch_methods"); // Creates the id column
	database.addColumn ("launch_methods", "name"                 , Database::dataTypeString   );
	database.addColumn ("launch_methods", "short_name"           , Database::dataTypeString   );
	database.addColumn ("launch_methods", "log_string"           , Database::dataTypeString   );
	database.addColumn ("launch_methods", "keyboard_shortcut"    , Database::dataTypeCharacter);
	// Use a string type because SQLite does not support enums
	database.addColumn ("launch_methods", "type"                 , Database::dataTypeString   );
	database.addColumn ("launch_methods", "towplane_registration", Database::dataTypeString   );
	database.addColumn ("launch_methods", "person_required"      , Database::dataTypeBoolean  );
	database.addColumn ("launch_methods", "comments"             , Database::dataTypeString   );

	try
	{
		database.transaction ();

		foreach (LaunchMethod launchMethod, opts.configuredLaunchMethods)
		{
			// Don't use the methods of Database or LaunchMethod - they use the
			// current schema, we need to use the schema after this migration.
			std::cout << "Importing launch method: " << launchMethod.toString () << std::endl;

			QSqlQuery query=database.prepareQuery (
				"INSERT INTO launch_methods"
				"(id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments)"
				"values (?,?,?,?,?,?,?,?,?)"
				);

			query.addBindValue (launchMethod.id);
			query.addBindValue (launchMethod.name);
			query.addBindValue (launchMethod.shortName);
			query.addBindValue (launchMethod.logString);
			query.addBindValue (launchMethod.keyboardShortcut);
			query.addBindValue (LaunchMethod::typeToDb (launchMethod.type));
			query.addBindValue (launchMethod.towplaneRegistration);
			query.addBindValue (launchMethod.personRequired);
			query.addBindValue (launchMethod.comments);

			database.executeQuery (query);
		}

		database.commit ();
	}
	catch (...)
	{
		database.rollback ();
		throw;
	}
}

void Migration_20100216135637_add_launch_methods::down ()
{
	database.dropTable ("launch_methods");
}
