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
//   - person/plane flying [at time]
//   - person used: person has user or flight

// Launch methods: count, add, list all, get by id, get by type, get towplane
// Method: get towplane (known from launch method or from flight's towplane id)

// Merge person:
//   - update flight pilot/copilot/towpilot, user person; should be in transaction
//   - check if a flight or user still referencecs this user
//   - delete "wrong" person

// Use database problems: access denied, database not found, server lost, connection error

// Ping

// DbTable:
QString DbColumn::type_string () const
	// Now why doesn't the MySQL library have function to do this?
{
	QString r="???";
	switch (type)
	{
		case FIELD_TYPE_TINY: r="TINYINT"; break;
		case FIELD_TYPE_SHORT: r="SMALLINT"; break;
		case FIELD_TYPE_LONG: r="INTEGER"; break;
		case FIELD_TYPE_INT24: r="MEDIUMINT"; break;
		case FIELD_TYPE_LONGLONG: r="BIGINT"; break;
		case FIELD_TYPE_DECIMAL: r="DECIMAL"; break;	// synonyms DEC, NUMERIC, FIXED
		case FIELD_TYPE_FLOAT: r="FLOAT"; break;
		case FIELD_TYPE_DOUBLE: r="DOUBLE"; break; // synonyms REAL, DOUBLE PRECISION
		case FIELD_TYPE_TIMESTAMP: r="TIMESTAMP"; break;
		case FIELD_TYPE_DATE: r="DATE"; break;
		case FIELD_TYPE_TIME: r="TIME"; break;
		case FIELD_TYPE_DATETIME: r="DATETIME"; break;
		case FIELD_TYPE_YEAR: r="YEAR"; break;
		case FIELD_TYPE_STRING: r="CHAR"; break;
		case FIELD_TYPE_VAR_STRING: r="VARCHAR"; break;
		case FIELD_TYPE_BLOB: r=binary?"BLOB":"TEXT"; break;
		case FIELD_TYPE_TINY_BLOB: r=binary?"TINYBLOB":"TINYTEXT"; break;
		case FIELD_TYPE_MEDIUM_BLOB: r=binary?"MEDIUMBLOB":"MEDIUMTEXT"; break;
		case FIELD_TYPE_LONG_BLOB: r=binary?"LONGBLOB":"LONGTEXT"; break;
		case FIELD_TYPE_SET: r="SET"; break;
		case FIELD_TYPE_NULL: r="NULL"; break;

		// The following are either not supported or not documented in the
		// MySQL Documention section 14.2.6
		case FIELD_TYPE_ENUM: r="ENUM"; break;
		case FIELD_TYPE_NEWDATE: r="NEWDATE"; break;
		case FIELD_TYPE_GEOMETRY: r="GEOMETRY"; break;
		// no default to allow the compiler to warn

		// More types we are not interested in
		case MYSQL_TYPE_VARCHAR: r="VARCHAR"; break;
		case MYSQL_TYPE_BIT: r="BIT"; break;
		case MYSQL_TYPE_NEWDECIMAL: r="NEWDECIMAL"; break;
	}

	if (length>0) r+="("+QString::number (length)+")";

	return r;
}

