#ifndef db_id_h
#define db_id_h

#include <mysql.h>

typedef my_ulonglong db_id;

enum db_object_type { ot_flight, ot_plane, ot_person, ot_none };
enum query_type_t { qt_create, qt_update, qt_delete, qt_list, qt_count_murx, qt_describe };

bool id_invalid (db_id id);
static const db_id invalid_id=0;

#endif

