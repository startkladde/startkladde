#ifndef _dataTypes_h
#define _dataTypes_h

#include <cstdio>
#include <QString>
#include <QList>
#include <string>

#include <mysql.h>

#include "src/logging/messages.h"


enum lengthSpecification { lsShort, lsTable, lsLong, lsWithShortcut, lsPrintout, lsPilotLog };
enum casus { cas_nominativ, cas_genitiv, cas_dativ, cas_akkusativ };

// TODO move to DbEvent
enum db_event_type { det_none, det_add, det_delete, det_change, det_refresh };
// TODO replace with templates
enum db_event_table { db_kein, db_alle, db_person, db_flug, db_flugzeug };

// Hack for until we have replaced db_event_table with templates
template<class T> db_event_table getDbEventTable ();





QString std2q (std::string);
std::string q2std (QString);

#endif

