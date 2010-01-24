#ifndef _DbEvent_h
#define _DbEvent_h

#include "src/dataTypes.h"
#include "src/db/dbTypes.h"

#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/Person.h"

// XXX
#define VALUE(x,z) case x: printf (z); break;
#define DEFAULT default: printf ("???"); break;

class DbEvent
{
	public:
		DbEvent ();
		DbEvent (db_event_type, db_event_table, db_id);
		void dump () const;

		template<class T> static db_event_table getDbEventTable () { return db_kein; }

		db_event_type type;
		db_event_table table;
		db_id id;
};

#endif

