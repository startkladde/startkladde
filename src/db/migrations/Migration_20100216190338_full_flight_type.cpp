#include "Migration_20100216190338_full_flight_type.h"

#include <iostream>

#include "src/db/Database.h"

Migration_20100216190338_full_flight_type::Migration_20100216190338_full_flight_type (Database &database):
	Migration (database)
{
}

Migration_20100216190338_full_flight_type::~Migration_20100216190338_full_flight_type ()
{
}

void Migration_20100216190338_full_flight_type::up ()
{
	database.changeColumnType ("flights", "type", Database::dataTypeString);
	std::cout << "Updating flight type values" << std::endl;
	updateValues (dirUp);
}

void Migration_20100216190338_full_flight_type::down ()
{
	std::cout << "Updating flight type values" << std::endl;
	updateValues (dirDown);
	database.changeColumnType ("flights", "type", Database::dataTypeInteger);
}

void Migration_20100216190338_full_flight_type::updateValue (int oldValue, const QString &newValue, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:   database.updateColumnValues ("flights", "type", oldValue, newValue); break;
		case dirDown: database.updateColumnValues ("flights", "type", newValue, oldValue); break;
	}
}

void Migration_20100216190338_full_flight_type::updateValues (Migration::Direction direction)
{
	updateValue (1, "?"             , direction); // None
	updateValue (2, "normal"        , direction);
	updateValue (3, "training_2"    , direction);
	updateValue (4, "training_1"    , direction);
	updateValue (7, "tow"           , direction);
	updateValue (6, "guest_private" , direction);
	updateValue (8, "guest_external", direction);
}
