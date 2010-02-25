#include "Migration_20100216122008_rename_tables.h"

Migration_20100216122008_rename_tables::Migration_20100216122008_rename_tables (Db::Interface::DatabaseInterface &databaseInterface):
	Migration (databaseInterface)
{
}

Migration_20100216122008_rename_tables::~Migration_20100216122008_rename_tables ()
{
}

void Migration_20100216122008_rename_tables::up ()
{
	renameTables (dirUp);
}

void Migration_20100216122008_rename_tables::down ()
{
	renameTables (dirDown);
}

void Migration_20100216122008_rename_tables::renameTable (const QString &oldName, const QString &newName, Direction direction)
{
	switch (direction)
	{
		case dirUp:   renameTable (oldName, newName); break;
		case dirDown: renameTable (newName, oldName); break;
	}
}

void Migration_20100216122008_rename_tables::renameTables (Direction direction)
{
	renameTable ("person"  , "people" , direction);
	renameTable ("flugzeug", "planes" , direction);
	renameTable ("flug"    , "flights", direction);
	renameTable ("user"    , "users"  , direction);
}
