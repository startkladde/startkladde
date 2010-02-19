#ifndef _DbEvent_h
#define _DbEvent_h

#include "src/dataTypes.h"
#include "src/db/dbId.h"

//#include "src/model/Plane.h"
//#include "src/model/Flight.h"
//#include "src/model/Person.h"
//#include "src/model/LaunchMethod.h"

// FIXME remove
#define VALUE(x,z) case x: std::cout << (z); break;
#define DEFAULT default: std::cout << ("???"); break;

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

