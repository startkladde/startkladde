#include "Migration_20100215172237_add_towpilot.h"

Migration_20100215172237_add_towpilot::Migration_20100215172237_add_towpilot (Db::Interface::DatabaseInterface &databaseInterface):
	Migration (databaseInterface)
{
}

Migration_20100215172237_add_towpilot::~Migration_20100215172237_add_towpilot ()
{
}

void Migration_20100215172237_add_towpilot::up ()
{
	addColumn ("flug"     , "towpilot", dataTypeId     (), "", true);
	addColumn ("flug"     , "tpvn",     dataTypeString (), "", true);
	addColumn ("flug"     , "tpnn",     dataTypeString (), "", true);

	addColumn ("flug_temp", "towpilot", dataTypeId     (), "", true);
	addColumn ("flug_temp", "tpvn",     dataTypeString (), "", true);
	addColumn ("flug_temp", "tpnn",     dataTypeString (), "", true);
}

void Migration_20100215172237_add_towpilot::down ()
{
	// Since the towpilot columns were allowed, we don't remove them
}

