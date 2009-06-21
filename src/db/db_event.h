#ifndef db_event_h
#define db_event_h

#include "src/data_types.h"
#include "src/db/db_types.h"

// XXX
#define VALUE(x,z) case x: printf (z); break;
#define DEFAULT default: printf ("???"); break;

class db_event
{
	public:
		db_event ();
		db_event (db_event_type, db_event_table, db_id);
		void dump ();

		db_event_type type;
		db_event_table table;
		db_id id;
};




#endif

