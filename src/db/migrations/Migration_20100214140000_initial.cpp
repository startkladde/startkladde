#include "Migration_20100214140000_initial.h"

Migration_20100214140000_initial::Migration_20100214140000_initial (Db::Interface::Interface &interface):
	Migration (interface)
{
}

Migration_20100214140000_initial::~Migration_20100214140000_initial ()
{
}

void Migration_20100214140000_initial::up (OperationMonitorInterface monitor)
{
	monitor.checkCanceled(); createTable  ("person", true); // Creates the id column
	monitor.checkCanceled(); addColumn ("person", "nachname"  , dataTypeString (), "", true);
	monitor.checkCanceled(); addColumn ("person", "vorname"   , dataTypeString (), "", true);
	monitor.checkCanceled(); addColumn ("person", "verein"    , dataTypeString (), "", true);
	monitor.checkCanceled(); addColumn ("person", "spitzname" , dataTypeString (), "", true);
	monitor.checkCanceled(); addColumn ("person", "vereins_id", dataTypeString (), "", true);
	monitor.checkCanceled(); addColumn ("person", "bemerkung" , dataTypeString (), "", true);

	createTableLike ("person", "person_temp", true);

	monitor.checkCanceled(); createTable ("flugzeug", true); // Creates the id column
	monitor.checkCanceled(); addColumn ("flugzeug", "kennzeichen",            dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flugzeug", "verein",                 dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flugzeug", "sitze",                  dataTypeInteger   (), "", true);
	monitor.checkCanceled(); addColumn ("flugzeug", "typ",                    dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flugzeug", "gattung",                dataTypeCharacter (), "", true);
	monitor.checkCanceled(); addColumn ("flugzeug", "wettbewerbskennzeichen", dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flugzeug", "bemerkung",              dataTypeString    (), "", true);

	monitor.checkCanceled(); createTableLike ("flugzeug", "flugzeug_temp", true);

	monitor.checkCanceled(); createTable ("flug", true); // Creates the id column
	// Crew and plane
	monitor.checkCanceled(); addColumn ("flug", "flugzeug",           dataTypeId        (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "pilot",              dataTypeId        (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "begleiter",          dataTypeId        (), "", true);
	// Flight settings
	monitor.checkCanceled(); addColumn ("flug", "typ",                dataTypeInteger   (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "modus",              dataTypeCharacter (), "", true);
	// Departure and landing
	monitor.checkCanceled(); addColumn ("flug", "status",             dataTypeInteger   (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "startart",           dataTypeId        (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "startort",           dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "zielort",            dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "anzahl_landungen",   dataTypeInteger   (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "startzeit",          dataTypeDatetime  (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "landezeit",          dataTypeDatetime  (), "", true);
	// Towflight
	monitor.checkCanceled(); addColumn ("flug", "towplane",           dataTypeId        (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "modus_sfz",          dataTypeCharacter (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "zielort_sfz",        dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "land_schlepp",       dataTypeDatetime  (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "towpilot",           dataTypeId        (), "", true);
	// Incomplete names
	monitor.checkCanceled(); addColumn ("flug", "pnn",                dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "pvn",                dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "bnn",                dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "bvn",                dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "tpnn",               dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "tpvn",               dataTypeString    (), "", true);
	// Comments
	monitor.checkCanceled(); addColumn ("flug", "bemerkung",          dataTypeString    (), "", true);
	monitor.checkCanceled(); addColumn ("flug", "abrechnungshinweis", dataTypeString    (), "", true);

	monitor.checkCanceled(); createTableLike ("flug", "flug_temp", true);

	monitor.checkCanceled(); createTable ("user", true); // Creates the id column
	monitor.checkCanceled(); addColumn ("user", "username",            dataTypeString  (), "NOT NULL", true); // TODO not null
	monitor.checkCanceled(); addColumn ("user", "password",            dataTypeString  (), ""        , true);
	monitor.checkCanceled(); addColumn ("user", "perm_club_admin",     dataTypeBoolean (), ""        , true);
	monitor.checkCanceled(); addColumn ("user", "perm_read_flight_db", dataTypeBoolean (), ""        , true);
	monitor.checkCanceled(); addColumn ("user", "club",                dataTypeString  (), ""        , true);
	monitor.checkCanceled(); addColumn ("user", "person",              dataTypeId      (), ""        , true);
}

void Migration_20100214140000_initial::down (OperationMonitorInterface monitor)
{
	dropTable ("person"  ); dropTable ("person_temp"  );
	dropTable ("flugzeug"); dropTable ("flugzeug_temp");
	dropTable ("flug"    ); dropTable ("flug_temp"    );
	dropTable ("user");
}
