#include "dbId.h"

bool id_valid (db_id id)
{
	return !id_invalid (id);
}

bool id_invalid (db_id id)
{
	return (id==0);
}
