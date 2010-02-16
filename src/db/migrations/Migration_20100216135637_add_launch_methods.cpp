#include "Migration_20100216135637_add_launch_methods.h"

#include "src/db/Database.h"

#include "src/model/LaunchType.h"
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

		foreach (LaunchType launchType, opts.configuredLaunchTypes)
		{
			// Don't use the methods of Database or LaunchType - they use the
			// current schema, we need to use the schema after this migration.
			std::cout << "Importing launch method: " << launchType.toString () << std::endl;

			QSqlQuery query=database.prepareQuery (
				"INSERT INTO launch_methods"
				"(id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments)"
				"values (?,?,?,?,?,?,?,?,?)"
				);

			query.addBindValue (launchType.id);
			query.addBindValue (launchType.description);
			query.addBindValue (launchType.short_description);
			query.addBindValue (launchType.logbook_string);
			query.addBindValue (launchType.accelerator);
			query.addBindValue (LaunchType::typeToDb (launchType.type));
			query.addBindValue (launchType.towplane);
			query.addBindValue (launchType.person_required);
			query.addBindValue (launchType.comments);

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
