#include "Migration_20100215221900_fix_data_types.h"

#include "src/db/Database.h"

Migration_20100215221900_fix_data_types::Migration_20100215221900_fix_data_types (Database &database):
	Migration (database)
{
}

Migration_20100215221900_fix_data_types::~Migration_20100215221900_fix_data_types ()
{
}

void Migration_20100215221900_fix_data_types::up ()
{
	changePeopleTable ("person");
	changePeopleTable ("person_temp");
	changePlanesTable ("flugzeug");
	changePlanesTable ("flugzeug_temp");
	changeFlightsTable ("flug");
	changeFlightsTable ("flug_temp");
	changeUsersTable ("user");
}

void Migration_20100215221900_fix_data_types::down ()
{
	// Don't change back
}

void Migration_20100215221900_fix_data_types::changePeopleTable (QString name)
{
	database.changeColumnType (name, "nachname"  , Database::dataTypeString);
	database.changeColumnType (name, "vorname"   , Database::dataTypeString);
	database.changeColumnType (name, "verein"    , Database::dataTypeString);
	database.changeColumnType (name, "spitzname" , Database::dataTypeString);
	database.changeColumnType (name, "vereins_id", Database::dataTypeString);
	database.changeColumnType (name, "bemerkung" , Database::dataTypeString);
}

void Migration_20100215221900_fix_data_types::changePlanesTable (QString name)
{
	database.changeColumnType (name, "kennzeichen",            Database::dataTypeString   );
	database.changeColumnType (name, "verein",                 Database::dataTypeString   );
	database.changeColumnType (name, "sitze",                  Database::dataTypeInteger  );
	database.changeColumnType (name, "typ",                    Database::dataTypeString   );
	database.changeColumnType (name, "gattung",                Database::dataTypeCharacter);
	database.changeColumnType (name, "wettbewerbskennzeichen", Database::dataTypeString   );
	database.changeColumnType (name, "bemerkung",              Database::dataTypeString   );
}

void Migration_20100215221900_fix_data_types::changeFlightsTable (QString name)
{
	database.changeColumnType (name, "flugzeug",           Database::dataTypeId       );
	database.changeColumnType (name, "pilot",              Database::dataTypeId       );
	database.changeColumnType (name, "begleiter",          Database::dataTypeId       );
	// Flight settings
	database.changeColumnType (name, "typ",                Database::dataTypeInteger  );
	database.changeColumnType (name, "modus",              Database::dataTypeCharacter);
	// Takeoff and landing
	database.changeColumnType (name, "status",             Database::dataTypeInteger  );
	database.changeColumnType (name, "startart",           Database::dataTypeId       );
	database.changeColumnType (name, "startort",           Database::dataTypeString   );
	database.changeColumnType (name, "zielort",            Database::dataTypeString   );
	database.changeColumnType (name, "anzahl_landungen",   Database::dataTypeInteger  );
	database.changeColumnType (name, "startzeit",          Database::dataTypeDatetime );
	database.changeColumnType (name, "landezeit",          Database::dataTypeDatetime );
	// Towflight
	database.changeColumnType (name, "towplane",           Database::dataTypeId       );
	database.changeColumnType (name, "modus_sfz",          Database::dataTypeCharacter);
	database.changeColumnType (name, "zielort_sfz",        Database::dataTypeString   );
	database.changeColumnType (name, "land_schlepp",       Database::dataTypeDatetime );
	database.changeColumnType (name, "towpilot",           Database::dataTypeId       );
	// Incomplete names
	database.changeColumnType (name, "pvn",                Database::dataTypeString   );
	database.changeColumnType (name, "pnn",                Database::dataTypeString   );
	database.changeColumnType (name, "bvn",                Database::dataTypeString   );
	database.changeColumnType (name, "bnn",                Database::dataTypeString   );
	database.changeColumnType (name, "tpvn",               Database::dataTypeString   );
	database.changeColumnType (name, "tpnn",               Database::dataTypeString   );
	// Comments
	database.changeColumnType (name, "bemerkung",          Database::dataTypeString   );
	database.changeColumnType (name, "abrechnungshinweis", Database::dataTypeString   );
}

void Migration_20100215221900_fix_data_types::changeUsersTable (QString name)
{
	database.changeColumnType (name, "username",            Database::dataTypeString ); // TODO not null
	database.changeColumnType (name, "password",            Database::dataTypeString ); // TODO long enough?
	database.changeColumnType (name, "perm_club_admin",     Database::dataTypeBoolean);
	database.changeColumnType (name, "perm_read_flight_db", Database::dataTypeBoolean);
	database.changeColumnType (name, "club",                Database::dataTypeString );
	database.changeColumnType (name, "person",              Database::dataTypeId     );
}
