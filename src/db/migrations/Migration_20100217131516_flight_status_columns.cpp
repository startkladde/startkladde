#include "Migration_20100217131516_flight_status_columns.h"

#include <iostream>

#include "src/db/Database.h"

Migration_20100217131516_flight_status_columns::Migration_20100217131516_flight_status_columns (Database &database):
	Migration (database)
{
}

Migration_20100217131516_flight_status_columns::~Migration_20100217131516_flight_status_columns ()
{
}

void Migration_20100217131516_flight_status_columns::up ()
{
	database.addColumn ("flights", "departed"        , Database::dataTypeBoolean, "AFTER status");
	database.addColumn ("flights", "landed"          , Database::dataTypeBoolean, "AFTER departed");
	database.addColumn ("flights", "towflight_landed", Database::dataTypeBoolean, "AFTER landed");

	std::cout << "Updating status flags" << std::endl;
	database.executeQuery ("UPDATE flights SET "
		"departed        =IF(status&1, TRUE, FALSE),"
		"landed          =IF(status&2, TRUE, FALSE),"
		"towflight_landed=IF(status&4, TRUE, FALSE)");

	database.dropColumn ("flights", "status");
}

void Migration_20100217131516_flight_status_columns::down ()
{
	database.addColumn ("flights", "status", Database::dataTypeInteger, "AFTER departed");

	std::cout << "Updating status" << std::endl;
	database.executeQuery ("UPDATE flights SET status="
		"IF(departed        , 1, 0)+"
		"IF(landed          , 2, 0)+"
		"IF(towflight_landed, 4, 0)");

	database.dropColumn ("flights", "departed"        );
	database.dropColumn ("flights", "landed"          );
	database.dropColumn ("flights", "towflight_landed");
}
