#ifndef _dbTypes_h
#define _dbTypes_h

#include <mysql.h>

typedef my_ulonglong db_id;

// TODO get rid of db_object_type, replace with templates
enum db_object_type { ot_flight, ot_plane, ot_person, ot_none };
enum query_type_t { qt_create, qt_update, qt_delete, qt_list, qt_count_murx, qt_describe };

//// Hack for until we have replaced db_object_type with templates
//template<class T> db_object_type getDbObjectType ();

bool id_invalid (db_id id);
bool id_valid (db_id id);
static const db_id invalid_id=0;

#endif

