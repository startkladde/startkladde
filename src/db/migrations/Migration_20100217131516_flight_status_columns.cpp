#include "Migration_20100217131516_flight_status_columns.h"

#include <iostream>

Migration_20100217131516_flight_status_columns::Migration_20100217131516_flight_status_columns (Db::Interface::Interface &interface):
	Migration (interface)
{
}

Migration_20100217131516_flight_status_columns::~Migration_20100217131516_flight_status_columns ()
{
}

void Migration_20100217131516_flight_status_columns::up ()
{
	addColumn ("flights", "departed"        , dataTypeBoolean (), "AFTER status");
	addColumn ("flights", "landed"          , dataTypeBoolean (), "AFTER departed");
	addColumn ("flights", "towflight_landed", dataTypeBoolean (), "AFTER landed");

	std::cout << "Updating status flags" << std::endl;
	executeQuery (Db::Query ("UPDATE flights SET "
		"departed        =IF(status&1, TRUE, FALSE),"
		"landed          =IF(status&2, TRUE, FALSE),"
		"towflight_landed=IF(status&4, TRUE, FALSE)"));

	dropColumn ("flights", "status");
}

void Migration_20100217131516_flight_status_columns::down ()
{
	addColumn ("flights", "status", dataTypeInteger (), "AFTER departed");

	std::cout << "Updating status" << std::endl;
	executeQuery (Db::Query ("UPDATE flights SET status="
		"IF(departed        , 1, 0)+"
		"IF(landed          , 2, 0)+"
		"IF(towflight_landed, 4, 0)"));

	dropColumn ("flights", "departed"        );
	dropColumn ("flights", "landed"          );
	dropColumn ("flights", "towflight_landed");
}
