#ifndef DBID_H_
#define DBID_H_

/**
 * This is not part of Database because a lot of classes (e. g. all Entity
 * subclasses) depend on this type, but don't necessarily depend on Database.
 * If this were defined in Database.h, almost everything would have to be
 * rebuilt when Database is changed.
 */

#include <QtGlobal>

typedef quint32 db_id;

bool id_invalid (db_id id);
bool id_valid (db_id id);
static const db_id invalid_id=0;

#endif
