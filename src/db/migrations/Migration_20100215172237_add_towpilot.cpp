#include "Migration_20100215172237_add_towpilot.h"

#include "src/db/Database.h"

Migration_20100215172237_add_towpilot::Migration_20100215172237_add_towpilot (Database &database):
	Migration (database)
{
}

Migration_20100215172237_add_towpilot::~Migration_20100215172237_add_towpilot ()
{
}

void Migration_20100215172237_add_towpilot::up ()
{
	database.addColumn ("flug"     , "towpilot", Database::dataTypeId    , "", true);
	database.addColumn ("flug"     , "tpvn",     Database::dataTypeString, "", true);
	database.addColumn ("flug"     , "tpnn",     Database::dataTypeString, "", true);

	database.addColumn ("flug_temp", "towpilot", Database::dataTypeId    , "", true);
	database.addColumn ("flug_temp", "tpvn",     Database::dataTypeString, "", true);
	database.addColumn ("flug_temp", "tpnn",     Database::dataTypeString, "", true);
}

void Migration_20100215172237_add_towpilot::down ()
{
	// Since the towpilot columns were allowed, we don't remove them
}

