#include "Migration_20100214140000_initial.h"

Migration_20100214140000_initial::Migration_20100214140000_initial (Db::Interface::DefaultInterface &interface):
	Migration (interface)
{
}

Migration_20100214140000_initial::~Migration_20100214140000_initial ()
{
}

void Migration_20100214140000_initial::up ()
{
	createTable  ("person", true); // Creates the id column
	addColumn ("person", "nachname"  , dataTypeString (), "", true);
	addColumn ("person", "vorname"   , dataTypeString (), "", true);
	addColumn ("person", "verein"    , dataTypeString (), "", true);
	addColumn ("person", "spitzname" , dataTypeString (), "", true);
	addColumn ("person", "vereins_id", dataTypeString (), "", true);
	addColumn ("person", "bemerkung" , dataTypeString (), "", true);

	createTableLike ("person", "person_temp", true);

	createTable ("flugzeug", true); // Creates the id column
	addColumn ("flugzeug", "kennzeichen",            dataTypeString    (), "", true);
	addColumn ("flugzeug", "verein",                 dataTypeString    (), "", true);
	addColumn ("flugzeug", "sitze",                  dataTypeInteger   (), "", true);
	addColumn ("flugzeug", "typ",                    dataTypeString    (), "", true);
	addColumn ("flugzeug", "gattung",                dataTypeCharacter (), "", true);
	addColumn ("flugzeug", "wettbewerbskennzeichen", dataTypeString    (), "", true);
	addColumn ("flugzeug", "bemerkung",              dataTypeString    (), "", true);

	createTableLike ("flugzeug", "flugzeug_temp", true);

	createTable ("flug", true); // Creates the id column
	// Crew and plane
	addColumn ("flug", "flugzeug",           dataTypeId        (), "", true);
	addColumn ("flug", "pilot",              dataTypeId        (), "", true);
	addColumn ("flug", "begleiter",          dataTypeId        (), "", true);
	// Flight settings
	addColumn ("flug", "typ",                dataTypeInteger   (), "", true);
	addColumn ("flug", "modus",              dataTypeCharacter (), "", true);
	// Takeoff and landing
	addColumn ("flug", "status",             dataTypeInteger   (), "", true);
	addColumn ("flug", "startart",           dataTypeId        (), "", true);
	addColumn ("flug", "startort",           dataTypeString    (), "", true);
	addColumn ("flug", "zielort",            dataTypeString    (), "", true);
	addColumn ("flug", "anzahl_landungen",   dataTypeInteger   (), "", true);
	addColumn ("flug", "startzeit",          dataTypeDatetime  (), "", true);
	addColumn ("flug", "landezeit",          dataTypeDatetime  (), "", true);
	// Towflight
	addColumn ("flug", "towplane",           dataTypeId        (), "", true);
	addColumn ("flug", "modus_sfz",          dataTypeCharacter (), "", true);
	addColumn ("flug", "zielort_sfz",        dataTypeString    (), "", true);
	addColumn ("flug", "land_schlepp",       dataTypeDatetime  (), "", true);
	addColumn ("flug", "towpilot",           dataTypeId        (), "", true);
	// Incomplete names
	addColumn ("flug", "pvn",                dataTypeString    (), "", true);
	addColumn ("flug", "pnn",                dataTypeString    (), "", true);
	addColumn ("flug", "bvn",                dataTypeString    (), "", true);
	addColumn ("flug", "bnn",                dataTypeString    (), "", true);
	addColumn ("flug", "tpvn",               dataTypeString    (), "", true);
	addColumn ("flug", "tpnn",               dataTypeString    (), "", true);
	// Comments
	addColumn ("flug", "bemerkung",          dataTypeString    (), "", true);
	addColumn ("flug", "abrechnungshinweis", dataTypeString    (), "", true);

	createTableLike ("flug", "flug_temp", true);

	createTable ("user", true); // Creates the id column
	addColumn ("user", "username",            dataTypeString  (), "NOT NULL", true); // TODO not null
	addColumn ("user", "password",            dataTypeString  (), ""        , true);
	addColumn ("user", "perm_club_admin",     dataTypeBoolean (), ""        , true);
	addColumn ("user", "perm_read_flight_db", dataTypeBoolean (), ""        , true);
	addColumn ("user", "club",                dataTypeString  (), ""        , true);
	addColumn ("user", "person",              dataTypeId      (), ""        , true);
}

void Migration_20100214140000_initial::down ()
{
	dropTable ("person"  ); dropTable ("person_temp"  );
	dropTable ("flugzeug"); dropTable ("flugzeug_temp");
	dropTable ("flug"    ); dropTable ("flug_temp"    );
	dropTable ("user");
}
