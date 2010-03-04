#include "Migration_20100215211913_drop_old_columns.h"

Migration_20100215211913_drop_old_columns::Migration_20100215211913_drop_old_columns (Db::Interface::Interface &interface):
	Migration (interface)
{
}

Migration_20100215211913_drop_old_columns::~Migration_20100215211913_drop_old_columns ()
{
}

void Migration_20100215211913_drop_old_columns::up (OperationMonitorInterface monitor)
{
	dropColumn ("flug"  , "editierbar", true);
	dropColumn ("flug"  , "verein"    , true);
	dropColumn ("person", "bwlv"      , true);
}

void Migration_20100215211913_drop_old_columns::down (OperationMonitorInterface monitor)
{
	// Don't change back
}
