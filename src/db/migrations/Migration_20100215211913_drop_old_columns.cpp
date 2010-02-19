#include "Migration_20100215211913_drop_old_columns.h"

Migration_20100215211913_drop_old_columns::Migration_20100215211913_drop_old_columns (Database &database):
	Migration (database)
{
}

Migration_20100215211913_drop_old_columns::~Migration_20100215211913_drop_old_columns ()
{
}

void Migration_20100215211913_drop_old_columns::up ()
{
	dropColumn ("flug"  , "editierbar", true);
	dropColumn ("flug"  , "verein"    , true);
	dropColumn ("person", "bwlv"      , true);
}

void Migration_20100215211913_drop_old_columns::down ()
{
	// Don't change back
}
