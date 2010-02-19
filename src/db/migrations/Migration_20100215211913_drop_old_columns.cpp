#include "Migration_20100215211913_drop_old_columns.h"

#include "src/db/Database.h"

Migration_20100215211913_drop_old_columns::Migration_20100215211913_drop_old_columns (Database &database):
	Migration (database)
{
}

Migration_20100215211913_drop_old_columns::~Migration_20100215211913_drop_old_columns ()
{
}

void Migration_20100215211913_drop_old_columns::up ()
{
	database.dropColumn ("flug"  , "editierbar", true);
	database.dropColumn ("flug"  , "verein"    , true);
	database.dropColumn ("person", "bwlv"      , true);
}

void Migration_20100215211913_drop_old_columns::down ()
{
	// Don't change back
}
