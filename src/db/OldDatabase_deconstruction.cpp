using namespace std;

// For blob/text columns: charset ok?

// Es sollten mehr Signale emittiert werden.
//   - connection_established
//   - connection_lost
//   - start_query
//   - start_data
//   - end_data


// The text used in the database for null dates.
const QString db_null_time="0000-00-00 00:00:00";
// The format QString for to_string and strptime for converting to/from the
// database. Must be parsable, so don't use %0Y. This is the default anyway.
const char *db_time_format="%Y-%m-%d %H:%M:%S";

// Version:
//   - mysql_get_client_version
//   - mysql_thread_safe==1
//   - <50112: timeouts may not work correctly


dbTable OldDatabase::get_table_information (const QString table_name, bool resolve_like) const
{
	// BLOB is used to get TEXT
	dbTable r (table_name);

	DbColumn id_column ("id", FIELD_TYPE_LONGLONG, 0, "");
	id_column.auto_increment=true;
	id_column.not_null=true;

	if (table_name.length ()>5 && table_name.endsWith ("_temp"))
	{
		r.like_table=table_name.left (table_name.length ()-5);
	}
	else if (table_name=="flug")
	{
		//                              name,                 type,                  length, default
		r.columns.push_back (id_column);
		r.columns.push_back (DbColumn ("pilot",              FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (DbColumn ("begleiter",          FIELD_TYPE_LONGLONG,    0, "NULL"));
		if (opts.record_towpilot)
		{
			r.columns.push_back (DbColumn ("towpilot",          FIELD_TYPE_LONGLONG,    0, "NULL"));
		}
		r.columns.push_back (DbColumn ("startort",           FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("zielort",            FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("anzahl_landungen",   FIELD_TYPE_LONGLONG,    0, "0"));
		r.columns.push_back (DbColumn ("startzeit",          FIELD_TYPE_DATETIME,    0, "NULL"));
		r.columns.push_back (DbColumn ("landezeit",          FIELD_TYPE_DATETIME,    0, "NULL"));
		r.columns.push_back (DbColumn ("startart",           FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (DbColumn ("land_schlepp",       FIELD_TYPE_DATETIME,    0, "NULL"));
		r.columns.push_back (DbColumn ("typ",                FIELD_TYPE_TINY,        0, "NULL"));
		r.columns.push_back (DbColumn ("bemerkung",          FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("editierbar",         FIELD_TYPE_TINY,        1, "NULL"));
		r.columns.push_back (DbColumn ("verein",             FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("flugzeug",           FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (DbColumn ("status",             FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (DbColumn ("modus",              FIELD_TYPE_STRING,      1, ""));
		r.columns.push_back (DbColumn ("pvn",                FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("pnn",                FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("bvn",                FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("bnn",                FIELD_TYPE_BLOB,        0, ""));
		if (opts.record_towpilot)
		{
			r.columns.push_back (DbColumn ("tpvn",                FIELD_TYPE_BLOB,        0, ""));
			r.columns.push_back (DbColumn ("tpnn",                FIELD_TYPE_BLOB,        0, ""));
		}
		r.columns.push_back (DbColumn ("modus_sfz",          FIELD_TYPE_STRING,      1, ""));
		r.columns.push_back (DbColumn ("zielort_sfz",        FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("abrechnungshinweis", FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (DbColumn ("towplane",           FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.primary_key="id";
		r.unique_keys.push_back ("id");
	}
	else if (table_name=="flugzeug")
	{
		//                              name,                     type,               length, default
		r.columns.push_back (id_column);
		r.columns.push_back (DbColumn ("kennzeichen",            FIELD_TYPE_VAR_STRING, 255, ""));
		r.columns.push_back (DbColumn ("verein",                 FIELD_TYPE_BLOB,         0, ""));
		r.columns.push_back (DbColumn ("sitze",                  FIELD_TYPE_SHORT,        0, "0"));
		r.columns.push_back (DbColumn ("typ",                    FIELD_TYPE_BLOB,         0, ""));
		r.columns.push_back (DbColumn ("gattung",                FIELD_TYPE_STRING,       1, ""));
		r.columns.push_back (DbColumn ("wettbewerbskennzeichen", FIELD_TYPE_BLOB,         0, ""));
		r.columns.push_back (DbColumn ("bemerkung",              FIELD_TYPE_BLOB,         0, ""));
		r.primary_key="id";
		r.unique_keys.push_back ("id");
		r.unique_keys.push_back ("kennzeichen");
	}
	else if (table_name=="person")
	{
		//                              name,         type,            length, default
		r.columns.push_back (id_column);
		r.columns.push_back (DbColumn ("nachname",   FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (DbColumn ("vorname",    FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (DbColumn ("verein",     FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (DbColumn ("bwlv",       FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (DbColumn ("spitzname",  FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (DbColumn ("vereins_id", FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (DbColumn ("bemerkung",  FIELD_TYPE_BLOB, 0,      ""));

		r.primary_key="id";
		r.unique_keys.push_back ("id");
	}
	else if (table_name=="user")
	{
		// TODO: the TEXT (BLOB) column can have length "0" (meaning
		// "variable"), but it cannot be used as a key without specifying the
		// key length. DbColumn does not support specifying a key length, so
		// the field length must be used. Change this.
		//                              name,                  type,            length, default .flags
		r.columns.push_back (DbColumn ("username",            FIELD_TYPE_VAR_STRING,  255,   "" ).set_not_null (true));
		r.columns.push_back (DbColumn ("password",            FIELD_TYPE_BLOB,        0,     "" ));
		r.columns.push_back (DbColumn ("perm_club_admin",     FIELD_TYPE_TINY,        1,     "0"));	// bool is not supported by MySQL
		r.columns.push_back (DbColumn ("perm_read_flight_db", FIELD_TYPE_TINY,        1,     "0"));	// bool is not supported by MySQL
		r.columns.push_back (DbColumn ("club",                FIELD_TYPE_BLOB,        0,     ""));
		r.columns.push_back (DbColumn ("person",              FIELD_TYPE_LONGLONG,    0,     "0"));

		r.primary_key="username";
		r.unique_keys.push_back ("username");
	}
}

// Check usable (see also admin_functions):
//   - connect
//   - use db
//   - all required tables exist (list tables and compare with required list)
//   - tables are writeable
//   - check columns present and correct types



// More queries we may need:
//   - grant $permissions on $target to $user [identified by "password"]
//   - create database [if not exists] $name
//   - show tables
//   - listing columns: select * from $table where false (oder describe/show columns)
//   - alter table $table add column $spec
//   - alter table $table modify column $spec

// Default sorting columns:
//   - flight: effdate (better sort in client)
//   - plane: kennzeichen
//   - person: nachname,vorname,verein



// Time to string: to_string (db_time_format, tz_utc, 20, true);
// String to time: parse (text, tz_utc, db_time_format);





// Database errors:
//   - CR_SERVER_LOST: timeout (also happens if reconnect fails)
//   - CR_CONN_HOST_ERROR, CR_CONNECTION_ERROR: connection failed
//   - Query failed

// "Currently flying" should be named "still to land" (more flying than total flights with prepared incoming flights)

// Conditions:
//   - flight person: pilot or copilot (or towpilot!)
//   - on date: date_format(startzeit, '%Y-%m-%d')=...






// Methods:
//   - get pilots and copilots from flights satisfying (condition)
//   - person/plane flying [at time]
//   - get plane by registration
//   - list airfields
//   - list first names
//   - list last names
//   - list accounting notes
//   - list registrations
//   - list plane types
//   - list clubs (from flights and people)
//   - person used: person has user or flight

// Launch types: count, add, list all, get by id, get by type, get towplane
// Method: get towplane (known from launch type or from flight's towplane id)

// Merge person:
//   - update flight pilot/copilot/towpilot, user person; should be in transaction
//   - check if a flight or user still referencecs this user
//   - delete "wrong" person

// Importing: see sk_web, it's a bit complicated

// Use database problems: access denied, database not found, server lost, connection error

// Ping

