using namespace std;

// The text used in the database for null dates.
const QString db_null_time="0000-00-00 00:00:00";
// The format QString for to_string and strptime for converting to/from the
// database. Must be parsable, so don't use %0Y. This is the default anyway.
const char *db_time_format="%Y-%m-%d %H:%M:%S";

// Check usable (see also admin_functions):
//   - connect
//   - use db
//   - all required tables exist (list tables and compare with required list)
//   - tables are writeable
//   - check columns present and correct types



// More queries we may need:
//   - grant $permissions on $target to $user [identified by "password"]
//   - listing columns: select * from $table where false (oder describe/show columns)

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

// Method: get towplane (known from launch method or from flight's towplane id)

// Merge person:
//   - update flight pilot/copilot/towpilot, user person; should be in transaction
//   - check if a flight or user still referencecs this user
//   - delete "wrong" person
