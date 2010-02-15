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
	// FIXME
}

void Migration_20100215221900_fix_data_types::down ()
{
	// Don't change back
}

