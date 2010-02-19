#include "Migration_20100216180053_full_plane_category.h"

#include "src/db/Database.h"

Migration_20100216180053_full_plane_category::Migration_20100216180053_full_plane_category (Database &database):
	Migration (database)
{
}

Migration_20100216180053_full_plane_category::~Migration_20100216180053_full_plane_category ()
{
}

void Migration_20100216180053_full_plane_category::up ()
{
	database.changeColumnType ("planes", "category", Database::dataTypeString);
	updateValues (dirUp);
}

void Migration_20100216180053_full_plane_category::down ()
{
	updateValues (dirDown);
	database.changeColumnType ("planes", "category", Database::dataTypeCharacter);
}

void Migration_20100216180053_full_plane_category::updateValue (const QString &oldValue, const QString &newValue, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:   database.updateColumnValues ("planes", "category", oldValue, newValue); break;
		case dirDown: database.updateColumnValues ("planes", "category", newValue, oldValue); break;
	}
}

void Migration_20100216180053_full_plane_category::updateValues (Migration::Direction direction)
{
	updateValue ("e", "single_engine", direction);
	updateValue ("1", "glider"       , direction);
	updateValue ("k", "motorglider"  , direction);
	updateValue ("m", "ultralight"   , direction);
	updateValue ("s", "other"        , direction);
}
