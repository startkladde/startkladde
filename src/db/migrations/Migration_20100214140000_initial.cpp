#include "Migration_20100214140000_initial.h"

#include "src/db/Database.h"

Migration_20100214140000_initial::Migration_20100214140000_initial (Database &database):
	Migration (database)
{
}

Migration_20100214140000_initial::~Migration_20100214140000_initial ()
{
}

void Migration_20100214140000_initial::up ()
{
	database.createTable  ("person", true); // Creates the id column
	database.addColumn ("person", "nachname"  , Database::dataTypeString, "", true);
	database.addColumn ("person", "vorname"   , Database::dataTypeString, "", true);
	database.addColumn ("person", "verein"    , Database::dataTypeString, "", true);
	database.addColumn ("person", "spitzname" , Database::dataTypeString, "", true);
	database.addColumn ("person", "vereins_id", Database::dataTypeString, "", true);
	database.addColumn ("person", "bemerkung" , Database::dataTypeString, "", true);

	database.createTableLike ("person", "person_temp", true);

	database.createTable ("flugzeug", true); // Creates the id column
	database.addColumn ("flugzeug", "kennzeichen",            Database::dataTypeString   , "", true);
	database.addColumn ("flugzeug", "verein",                 Database::dataTypeString   , "", true);
	database.addColumn ("flugzeug", "sitze",                  Database::dataTypeInteger  , "", true);
	database.addColumn ("flugzeug", "typ",                    Database::dataTypeString   , "", true);
	database.addColumn ("flugzeug", "gattung",                Database::dataTypeCharacter, "", true);
	database.addColumn ("flugzeug", "wettbewerbskennzeichen", Database::dataTypeString   , "", true);
	database.addColumn ("flugzeug", "bemerkung",              Database::dataTypeString   , "", true);

	database.createTableLike ("flugzeug", "flugzeug_temp", true);

	database.createTable ("flug", true); // Creates the id column
	// Crew and plane
	database.addColumn ("flug", "flugzeug",           Database::dataTypeId       , "", true);
	database.addColumn ("flug", "pilot",              Database::dataTypeId       , "", true);
	database.addColumn ("flug", "begleiter",          Database::dataTypeId       , "", true);
	// Flight settings
	database.addColumn ("flug", "typ",                Database::dataTypeInteger  , "", true);
	database.addColumn ("flug", "modus",              Database::dataTypeCharacter, "", true);
	// Takeoff and landing
	database.addColumn ("flug", "status",             Database::dataTypeInteger  , "", true);
	database.addColumn ("flug", "startart",           Database::dataTypeId       , "", true);
	database.addColumn ("flug", "startort",           Database::dataTypeString   , "", true);
	database.addColumn ("flug", "zielort",            Database::dataTypeString   , "", true);
	database.addColumn ("flug", "anzahl_landungen",   Database::dataTypeInteger  , "", true);
	database.addColumn ("flug", "startzeit",          Database::dataTypeDatetime , "", true);
	database.addColumn ("flug", "landezeit",          Database::dataTypeDatetime , "", true);
	// Towflight
	database.addColumn ("flug", "towplane",           Database::dataTypeId       , "", true);
	database.addColumn ("flug", "modus_sfz",          Database::dataTypeCharacter, "", true);
	database.addColumn ("flug", "zielort_sfz",        Database::dataTypeString   , "", true);
	database.addColumn ("flug", "land_schlepp",       Database::dataTypeDatetime , "", true);
	database.addColumn ("flug", "towpilot",           Database::dataTypeId       , "", true);
	// Incomplete names
	database.addColumn ("flug", "pvn",                Database::dataTypeString   , "", true);
	database.addColumn ("flug", "pnn",                Database::dataTypeString   , "", true);
	database.addColumn ("flug", "bvn",                Database::dataTypeString   , "", true);
	database.addColumn ("flug", "bnn",                Database::dataTypeString   , "", true);
	database.addColumn ("flug", "tpvn",               Database::dataTypeString   , "", true);
	database.addColumn ("flug", "tpnn",               Database::dataTypeString   , "", true);
	// Comments
	database.addColumn ("flug", "bemerkung",          Database::dataTypeString   , "", true);
	database.addColumn ("flug", "abrechnungshinweis", Database::dataTypeString   , "", true);

	database.createTableLike ("flug", "flug_temp", true);

	database.createTable ("user", true); // Creates the id column
	database.addColumn ("user", "username",            Database::dataTypeString , "NOT NULL", true); // TODO not null
	database.addColumn ("user", "password",            Database::dataTypeString , ""        , true);
	database.addColumn ("user", "perm_club_admin",     Database::dataTypeBoolean, ""        , true);
	database.addColumn ("user", "perm_read_flight_db", Database::dataTypeBoolean, ""        , true);
	database.addColumn ("user", "club",                Database::dataTypeString , ""        , true);
	database.addColumn ("user", "person",              Database::dataTypeId     , ""        , true);
}

void Migration_20100214140000_initial::down ()
{
	database.dropTable ("person"  ); database.dropTable ("person_temp"  );
	database.dropTable ("flugzeug"); database.dropTable ("flugzeug_temp");
	database.dropTable ("flug"    ); database.dropTable ("flug_temp"    );
	database.dropTable ("user");
}
