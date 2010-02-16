#include "Migration_20100216122008_rename_tables.h"

#include <QString>

#include "src/db/Database.h"

Migration_20100216122008_rename_tables::Migration_20100216122008_rename_tables (Database &database):
	Migration (database)
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
		case dirUp:   database.renameTable (oldName, newName); break;
		case dirDown: database.renameTable (newName, oldName); break;
	}
}

void Migration_20100216122008_rename_tables::renameTables (Direction direction)
{
	renameTable ("person"  , "people" , direction);
	renameTable ("flugzeug", "planes" , direction);
	renameTable ("flug"    , "flights", direction);
	renameTable ("user"    , "users"  , direction);
}
