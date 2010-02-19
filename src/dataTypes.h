#ifndef _dataTypes_h
#define _dataTypes_h


enum lengthSpecification { lsShort, lsTable, lsLong, lsWithShortcut, lsPrintout, lsPilotLog };

// TODO move to DbEvent
enum db_event_type { det_none, det_add, det_delete, det_change, det_refresh };
// TODO replace with templates
enum db_event_table { db_kein, db_alle, db_person, db_flug, db_launch_method, db_flugzeug };

// Hack for until we have replaced db_event_table with templates
template<class T> db_event_table getDbEventTable ();






#endif

