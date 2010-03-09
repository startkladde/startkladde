#include "Migration_20100215172237_add_towpilot.h"

Migration_20100215172237_add_towpilot::Migration_20100215172237_add_towpilot (Interface &interface):
	Migration (interface)
{
}

Migration_20100215172237_add_towpilot::~Migration_20100215172237_add_towpilot ()
{
}

void Migration_20100215172237_add_towpilot::up (OperationMonitorInterface monitor)
{
	addColumn ("flug"     , "towpilot", dataTypeId     (), "", true);
	addColumn ("flug"     , "tpvn",     dataTypeString (), "", true);
	addColumn ("flug"     , "tpnn",     dataTypeString (), "", true);

	addColumn ("flug_temp", "towpilot", dataTypeId     (), "", true);
	addColumn ("flug_temp", "tpvn",     dataTypeString (), "", true);
	addColumn ("flug_temp", "tpnn",     dataTypeString (), "", true);
}

void Migration_20100215172237_add_towpilot::down (OperationMonitorInterface monitor)
{
	// Since the towpilot columns were allowed, we don't remove them
}

