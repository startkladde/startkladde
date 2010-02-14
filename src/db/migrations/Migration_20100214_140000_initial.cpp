#include "Migration_20100214_140000_initial.h"

#include "src/db/Database.h"

Migration_20100214_140000_initial::Migration_20100214_140000_initial (Database &database):
	Migration (database)
{
}

Migration_20100214_140000_initial::~Migration_20100214_140000_initial ()
{
}

void Migration_20100214_140000_initial::up ()
{
	database.createTable  ("person");
	database.addColumn ("person", "nachname"  , Database::dataTypeString);
	database.addColumn ("person", "vorname"   , Database::dataTypeString);
	database.addColumn ("person", "verein"    , Database::dataTypeString);
	database.addColumn ("person", "spitzname" , Database::dataTypeString);
	database.addColumn ("person", "vereins_id", Database::dataTypeString);
	database.addColumn ("person", "bemerkung" , Database::dataTypeString);

	database.createTableLike ("person", "person_temp");

	database.createTable ("flugzeug");
	database.addColumn ("flugzeug", "kennzeichen",            Database::dataTypeString   );
	database.addColumn ("flugzeug", "verein",                 Database::dataTypeString   );
	database.addColumn ("flugzeug", "sitze",                  Database::dataTypeInteger  );
	database.addColumn ("flugzeug", "typ",                    Database::dataTypeString   );
	database.addColumn ("flugzeug", "gattung",                Database::dataTypeCharacter);
	database.addColumn ("flugzeug", "wettbewerbskennzeichen", Database::dataTypeString   );
	database.addColumn ("flugzeug", "bemerkung",              Database::dataTypeString   );

	database.createTableLike ("flugzeug", "flugzeug_temp");

	database.createTable ("flug");
	// Crew and plane
	database.addColumn ("flug", "flugzeug",           Database::dataTypeId       );
	database.addColumn ("flug", "pilot",              Database::dataTypeId       );
	database.addColumn ("flug", "begleiter",          Database::dataTypeId       );
	// Flight settings
	database.addColumn ("flug", "typ",                Database::dataTypeInteger  );
	database.addColumn ("flug", "modus",              Database::dataTypeCharacter);
	// Takeoff and landing
	database.addColumn ("flug", "status",             Database::dataTypeInteger  );
	database.addColumn ("flug", "startart",           Database::dataTypeId       );
	database.addColumn ("flug", "startort",           Database::dataTypeString   );
	database.addColumn ("flug", "zielort",            Database::dataTypeString   );
	database.addColumn ("flug", "anzahl_landungen",   Database::dataTypeInteger  );
	database.addColumn ("flug", "startzeit",          Database::dataTypeDatetime );
	database.addColumn ("flug", "landezeit",          Database::dataTypeDatetime );
	// Towflight
	database.addColumn ("flug", "towplane",           Database::dataTypeId       );
	database.addColumn ("flug", "modus_sfz",          Database::dataTypeCharacter);
	database.addColumn ("flug", "zielort_sfz",        Database::dataTypeString   );
	database.addColumn ("flug", "land_schlepp",       Database::dataTypeDatetime );
	database.addColumn ("flug", "towpilot",           Database::dataTypeId       );
	// Incomplete names
	database.addColumn ("flug", "pvn",                Database::dataTypeString   );
	database.addColumn ("flug", "pnn",                Database::dataTypeString   );
	database.addColumn ("flug", "bvn",                Database::dataTypeString   );
	database.addColumn ("flug", "bnn",                Database::dataTypeString   );
	database.addColumn ("flug", "tpvn",               Database::dataTypeString   );
	database.addColumn ("flug", "tpnn",               Database::dataTypeString   );
	// Comments
	database.addColumn ("flug", "bemerkung",          Database::dataTypeString   );
	database.addColumn ("flug", "abrechnungshinweis", Database::dataTypeString   );

	database.createTableLike ("flug", "flug_temp");

	database.createTable ("user");
	database.addColumn ("user", "username",            Database::dataTypeString ); // TODO not null
	database.addColumn ("user", "password",            Database::dataTypeString ); // TODO long enough?
	database.addColumn ("user", "perm_club_admin",     Database::dataTypeBoolean);
	database.addColumn ("user", "perm_read_flight_db", Database::dataTypeBoolean);
	database.addColumn ("user", "club",                Database::dataTypeString );
	database.addColumn ("user", "person",              Database::dataTypeId     );
}

void Migration_20100214_140000_initial::down ()
{
	database.dropTable ("person"  ); database.dropTable ("person_temp"  );
	database.dropTable ("flugzeug"); database.dropTable ("flugzeug_temp");
	database.dropTable ("flug"    ); database.dropTable ("flug_temp"    );
	database.dropTable ("user");
}
