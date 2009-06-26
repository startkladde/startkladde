#include "sk_db.h"

#include <algorithm>
#include <assert.h>
#include <sstream>

#include <q3ptrlist.h> // XXX
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator
#define QValueListIterator Q3ValueListIterator

#include <mysqld_error.h>

#include "src/text.h"
#include "src/data_types.h"
#include "src/config/options.h"



/*
 * This will have to be redone some time.
 * What we learn from this implemenation:
 *   - If trying to (edit/delete/whatever) an object which proves not
 *     to be editable, an exception should be thrown. Normally it is
 *     up to the caller to check editability *before* calling such a
 *     function.
 *   - The edit/create object functions should honor the 'editable' flag if one
 *     exists and this is an admin DB.
 *     If not, when changing from fixed to editable and changing values at the
 *     same time, first both tables are updated, then the entry is copied and
 *     finally deleted from one of the tables.
 *   - MySQL functionality should be separated from sk_db functionality
 */

/*
 * M�gliche L�sung zur Spezifikation der Bedingung:
 *   - F�r jede Kombination, die man braucht, einen enum-Wert aufnehmen.
 *     Probleme:
 *       - sehr viele einzeln zu behandelnde Spezialf�lle
 *   - Den Listenfunktionen fertige Queries �bergeben, die durch make_condition
 *     und and_condition etc. zusammengebaut werden.
 *     Probleme:
 *       - Komplexe conditions zu bauen wird un�bersichtlich und f�hrt zu
 *         codeduplikation (+" AND "+...)/uneinheitlichen Queries
 *       - Codeduplikation, wenn gleiche Teile mehrfach gebraucht werden.
 */

// New database code:
//   - Edits database entries instead of deleting and recreating them
//   - Avoids code duplication (queries)
//   - Escaping is done centrally (avoids errors)
//   - Uses streams instead of printf (avoids errors, see the %llu error...)
//   - Row parsing uses row headers
//   - Compiles with -Wall -Werror

// Query construction template:
	//QString condition=make_condition (cond_, 0, NULL, NULL, NULL, NULL, NULL);
	//QString columns=query_column_list (ot_, false);
	//QString query=make_query (qt_list, ot_, condition, "", columns);

// list_flights call template:
	//return list_flights (list, cond_flight_, 0, NULL, NULL, NULL, NULL, NULL);

// When changing the database:
//   - row_to_object
//   - query_value_list
//   - query_column_list

// Sortierung in Personenlistfunktionen

// TODO: add functions for deleting from the non-_temp tables when we
// are admin

// TODO parameter checking

// TODO add "not editable" handling for all writing functions

// TODO iterator it; *it nur bei pointerlist

// TODO cond_ ist schlecht, besser crit_

// Nichts mit condition_t sollte public sein, weil nicht type safe
// und das Interface trotz allem beschissen ist.

// Die condition_t sollten als referenzen �bergeben werden

// TODO condition und columns evtl. doch innerhalb von make_query
// erzeugen?
//
// TODO reconnect, dazu m�ssen die Verbindungsdaten doch gespeichert werden.
// Dann nicht beim Start sofort connecten bzw. nicht abbrechen, wenn das nicht
// klappt.
// Au�erdem sollten mehr Signale emittiert werden.
//   - connection_established
//   - connection_lost
//   - start_query
//   - start_data
//   - end_data

// TODO statt cerr benutze messages.h, und untscheeide zwischen
// Programmfehlern (fehlendes case...) und Laufzeitfehlern (Verbindung
// kann nicht aufgebaut werden...)

// TODO: Unify bitmask<-> functions, enum<->db functions, data structure<->db functions

// TODO: centralize functions which contain information about other data structures

// TODO all functions should check all parameters

// TODO Funktionalit�t des Bedingung-Erzeugens in die condition_t-Klasse. au�erdem
// dort add_condition oder so dazuf�gen, sonst baut man jede einzelne
// kombination aus datuskriterium und anderem kriterium einzeln nach

//#define DEBUG

// get_* functions (should?) return db_err_not_found if not found, list_* functions don't

#ifdef DEBUG
#define DEB(text) debug_stream << text << std::endl;
#else
#define DEB(text)
#endif

#define DEBUG_OUTPUT(x) do { debug_stream << #x << "=" << x << std::endl; } while (false)

#define SEGFAULT *((int *)0)=0;

const int STATUS_GESTARTET=1;
const int STATUS_GELANDET=2;
const int STATUS_SFZ_GELANDET=4;


// TODO also add the table names as symbolic constants
// TODO use these constants everywhere
// TODO also use constants for the user tables
const QString column_name_editable="editable";
const QString column_name_count="count";
const QString column_name_id="id";
const QString column_name_flight_effdate="effdate";
const QString column_name_flug_pilot="pilot";
const QString column_name_flug_begleiter="begleiter";
const QString column_name_flug_towpilot="schlepppilot";
const QString column_name_flug_flugzeug="flugzeug";

// The text used in the database for null dates.
const QString db_null_time="0000-00-00 00:00:00";
// The format QString for to_string and strptime for converting to/from the
// database. Must be parsable, so don't use %0Y. This is the default anyway.
const char *db_time_format="%Y-%m-%d %H:%M:%S";

const QString query_separator=";";

// ***** import_message
bool sk_db::import_message::fatal (import_message_type t)
{
	switch (t)
	{
		case imt_first_name_missing: return true;
		case imt_last_name_missing: return true;
		case imt_duplicate_club_id: return true;
		case imt_duplicate_name_without_club_id: return true;
		case imt_club_id_not_found: return true;
		case imt_club_id_old_not_found: return true;
		case imt_club_id_not_unique: return true;
		case imt_multiple_own_persons_name: return true;
		case imt_multiple_own_editable_persons_name: return true;
		case imt_multiple_editable_persons_name: return true;
		case imt_club_mismatch: return false;
	}
	return true;
}

QString sk_db::import_message::description (bool extended) const
{
	// Message with a different extended message
#define MESSAGE_D(CASE, NUM_PERSONS, MESS, EXTMESS)	\
	case CASE:	\
		if ((NUM_PERSONS>=1) && !p1) return "Interner Fehler: p1 nicht angegeben";	\
		if ((NUM_PERSONS>=2) && !p2) return "Interner Fehler: p2 nicht angegeben";	\
		if (extended)	\
			return EXTMESS;	\
		else	\
			return MESS;	\
		break
	// Message with the same extended message
#define MESSAGE_S(CASE, NUM_PERSONS, MESS) MESSAGE_D (CASE, NUM_PERSONS, MESS, MESS)
	// Message with a QString appended for extended
#define MESSAGE_A(CASE, NUM_PERSONS, MESS, APPEND) MESSAGE_D (CASE, NUM_PERSONS, MESS, MESS+QString (APPEND))

	switch (type)
	{
		MESSAGE_S (imt_first_name_missing, 0, "Vorname nicht angegeben");
		MESSAGE_S (imt_last_name_missing, 0, "Nachname nicht angegeben");
		MESSAGE_A (imt_duplicate_club_id, 1, "Doppelte Vereins-ID", " \""+p1->club_id+"\"");
		MESSAGE_A (imt_duplicate_name_without_club_id, 1, "Doppelter Name ohne Vereins-ID", " \""+p1->text_name ()+"\"");
		MESSAGE_D (imt_club_id_not_found, 1, "Vereins-ID nicht gefunden", "Vereins-ID \""+p1->club_id+"\" nicht gefunden");
		MESSAGE_D (imt_club_id_old_not_found, 1, "Alte Vereins-ID nicht gefunden", "Alte Vereins-ID \""+p1->club_id_old+"\" nicht gefunden");
		MESSAGE_D (imt_club_id_not_unique, 1, "Vereins-ID mehrfach vorhanden", "Vereins-ID \""+p1->club_id+"\" nicht eindeutig");
		MESSAGE_D (imt_multiple_own_persons_name, 1, "Mehrere eigene Personen mit diesem Namen gefunden", "Mehrere eigene Personen mit dem Namen \""+p1->text_name ()+"\" gefunden");
		MESSAGE_D (imt_multiple_own_editable_persons_name, 1, "Mehrere eigene editierbare Personen mit diesem Namen gefunden", "Mehrere eigene editierbare Personen mit dem Namen \""+p1->text_name ()+"\" gefunden");
		MESSAGE_D (imt_multiple_editable_persons_name, 1, "Mehrere editierbare Personen mit diesem Namen gefunden", "Mehrere editierbare Personen mit dem Namen \""+p1->text_name ()+"\" gefunden");
		MESSAGE_S (imt_club_mismatch, 1, "Verein \""+text_value+"\" passt nicht");
	}
	return "Interner Fehler: unbehandelter Fall in sk_db::import_message::description";
#undef MESSAGE_A
#undef MESSAGE_D
#undef MESSAGE_S
}

// ***** sk_db

// Class and connection management
sk_db::sk_db (std::ostream &_debug_stream)
	:debug_stream (_debug_stream)
	/*
	 * Parameters:
	 *   - server: DNS name or IP address in dotted quad notation of the SQL
	 *     server.
	 *   - port: The port number of the SQL server. MySQL ignores this if the
	 *     server is localhost because a local server is connected via a unix
	 *     domain socket.
	 *   - username: the user name for logging into the SQL server.
	 *   - password: the password for logging into the SQL server.
	 *   - database: the SQL database to use
	 */
{
//	DEB ("database class created");

	port=0;
	mysql=NULL;
	connection_established=false;
	display_queries=false;
	silent=false;
	startarten.setAutoDelete (true);
	is_admin_db=false;
}

sk_db::~sk_db ()
{
//	DEB ("database class destroyed");

	disconnect ();
}

void sk_db::connect ()
	throw (ex_allocation_error, ex_connection_failed, ex_access_denied)
{
	disconnect ();

	// Initialize data structure
	mysql=mysql_init (NULL);
	if (!mysql)
	{
		debug_stream << "[db] mysql_init () failed in sk_db::connect ()" << std::endl;
		throw ex_allocation_error ();
	}

	// Use compression
	mysql_options (mysql, MYSQL_OPT_COMPRESS, NULL);

	// Connect to the server
	MYSQL *connection_result;

	if (!silent) std::cout << "Connecting: " << username.latin1() << "@" << server.latin1() << ":" << port << "/" << database.latin1() << std::endl;

	connection_result=mysql_real_connect (mysql, server.latin1(), username.latin1(), password.latin1(), NULL, port, NULL, 0);
	if (!connection_result)
	{
		debug_stream << "[db] mysql_real_connect () failed in sk_db::connect (): " << get_last_error () << std::endl;
		if (get_last_errno ()==ER_ACCESS_DENIED_ERROR)
			throw ex_access_denied (get_last_error ());
		else
			throw ex_connection_failed ();
	}
	connection_established=true;
	current_server=server;

//	DEB ("database connection established");
}

void sk_db::connect (QString _server, int _port, QString _username, QString _password)
	throw (ex_allocation_error, ex_connection_failed, ex_access_denied)
	// 0 on success
{
	set_connection_data (_server, _port, _username, _password);
	connect ();
}

void sk_db::use_db ()
	throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access)
{
	if (database.isEmpty ()) throw ex_parameter_error ("Datenbank nicht angegeben");

	if (display_queries) debug_stream << "USE " << database << std::endl;

	if (mysql_select_db (mysql, database.latin1())!=0)
	{
		if (get_last_errno ()==ER_DBACCESS_DENIED_ERROR)
			throw ex_insufficient_access (database);
		else if (get_last_errno ()==ER_BAD_DB_ERROR)
			throw ex_database_not_found (database);
		else
			throw ex_database_not_accessible ();
	}
}

void sk_db::use_db (QString _database)
	throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access)
{
	set_database (_database);
	use_db ();
}

void sk_db::set_connection_data (QString _server, int _port, QString _username, QString _password)
{
	server=_server;
	port=_port;
	username=_username;
	password=_password;
}

void sk_db::set_database (QString _database)
{
	database=_database;
}

void sk_db::set_user_data (QString _username, QString _password)
{
	username=_username;
	password=_password;
}

int sk_db::disconnect ()
{
	if (mysql)
	{
		mysql_close (mysql);
		mysql=NULL;
		connection_established=false;
		current_server="";
	}

	return db_ok;
}

QString sk_db::get_last_error () const
{
	if (mysql)
		return QString (mysql_error (mysql));
	else
		return "";
}

unsigned int sk_db::get_last_errno () const
{
	if (mysql)
		return mysql_errno (mysql);
	else
		return 0;
}

bool sk_db::connected () const
{
	return connection_established;
}

bool sk_db::alife () const
{
	return connected () && (mysql_ping (mysql)==0);
}

QString sk_db::db_error_description (int error, bool extended) const
{
	switch (error)
	{
		case db_ok: return "Kein Fehler"; break;
		case db_err_not_connected: return "Keine Datenbankverbindung"; break;
		//case db_err_allocation_failure: return "Speicherfehler"; break;
		//case db_err_connection_error: return "Verbindungsfehler"; break;
		case db_err_too_few_columns: return "Zu wenige Spalten"; break;
		case db_err_unhandled: return "Unbehandelter Fall"; break;
		case db_err_not_found: return "Nicht gefunden"; break;
		case db_err_mysql_error: return "mysql-Fehler"; break;
		case db_err_query_failed:
			if (extended)
				return "Query fehlgeschlagen:\n"+get_last_error ()+"\nQuery: "+get_last_query ();
			else
				return "Query fehlgeschlagen";
			break;
		case db_err_parameter_error: return "Parameterfehler"; break;
		case db_err_not_implemented: return "Nicht implementiert"; break;
		case db_err_not_editable: return "Nicht editierbar"; break;
		case db_err_multiple: return "Ergebnis nicht eindeutig"; break;
		case db_err_already_exists: return "Objekt exisitiert bereits"; break;
		case db_err_not_admin_db: return "Aministrationsverbindung ben�tigt"; break;
		default: return "Fehler "+QString::number (error); break;
	}
}



// Database metainformation
void sk_db::list_required_tables (QStringList &tables)
{
	// Tables that depend on another table must be listed after that table.
	tables.push_back ("flug");
	tables.push_back ("flug_temp");
	tables.push_back ("flugzeug");
	tables.push_back ("flugzeug_temp");
	tables.push_back ("person");
	tables.push_back ("person_temp");
	tables.push_back ("user");
}

void sk_db::list_required_writeable_tables (QStringList &tables)
{
	tables.push_back ("flug_temp");
	tables.push_back ("flugzeug_temp");
	tables.push_back ("person_temp");
}

db_table sk_db::get_table_information (const QString table_name, bool resolve_like) const
	/*
	 * Returns information required to create the table, like columns, column
	 * types, keys etc. in a db_table structure.
	 * Parameters:
	 *   - table_name: the name identifying the table
	 *   - resolve_like: whether to resolve "like tablename" tables immediately
	 *     instead of returning "like tablename".
	 * Return value:
	 *   - The table information
	 */
{
	// BLOB is used to get TEXT
	db_table r (table_name);

	db_column id_column ("id", FIELD_TYPE_LONGLONG, 0, "");
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
		r.columns.push_back (db_column ("pilot",              FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (db_column ("begleiter",          FIELD_TYPE_LONGLONG,    0, "NULL"));
		if (opts.record_towpilot)
		{
			r.columns.push_back (db_column ("towpilot",          FIELD_TYPE_LONGLONG,    0, "NULL"));
		}
		r.columns.push_back (db_column ("startort",           FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("zielort",            FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("anzahl_landungen",   FIELD_TYPE_LONGLONG,    0, "0"));
		r.columns.push_back (db_column ("startzeit",          FIELD_TYPE_DATETIME,    0, "NULL"));
		r.columns.push_back (db_column ("landezeit",          FIELD_TYPE_DATETIME,    0, "NULL"));
		r.columns.push_back (db_column ("startart",           FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (db_column ("land_schlepp",       FIELD_TYPE_DATETIME,    0, "NULL"));
		r.columns.push_back (db_column ("typ",                FIELD_TYPE_TINY,        0, "NULL"));
		r.columns.push_back (db_column ("bemerkung",          FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("editierbar",         FIELD_TYPE_TINY,        1, "NULL"));
		r.columns.push_back (db_column ("verein",             FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("flugzeug",           FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (db_column ("status",             FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.columns.push_back (db_column ("modus",              FIELD_TYPE_STRING,      1, ""));
		r.columns.push_back (db_column ("pvn",                FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("pnn",                FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("bvn",                FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("bnn",                FIELD_TYPE_BLOB,        0, ""));
		if (opts.record_towpilot)
		{
			r.columns.push_back (db_column ("tpvn",                FIELD_TYPE_BLOB,        0, ""));
			r.columns.push_back (db_column ("tpnn",                FIELD_TYPE_BLOB,        0, ""));
		}
		r.columns.push_back (db_column ("modus_sfz",          FIELD_TYPE_STRING,      1, ""));
		r.columns.push_back (db_column ("zielort_sfz",        FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("abrechnungshinweis", FIELD_TYPE_BLOB,        0, ""));
		r.columns.push_back (db_column ("towplane",           FIELD_TYPE_LONGLONG,    0, "NULL"));
		r.primary_key="id";
		r.unique_keys.push_back ("id");
	}
	else if (table_name=="flugzeug")
	{
		//                              name,                     type,               length, default
		r.columns.push_back (id_column);
		r.columns.push_back (db_column ("kennzeichen",            FIELD_TYPE_VAR_STRING, 255, ""));
		r.columns.push_back (db_column ("verein",                 FIELD_TYPE_BLOB,         0, ""));
		r.columns.push_back (db_column ("sitze",                  FIELD_TYPE_SHORT,        0, "0"));
		r.columns.push_back (db_column ("typ",                    FIELD_TYPE_BLOB,         0, ""));
		r.columns.push_back (db_column ("gattung",                FIELD_TYPE_STRING,       1, ""));
		r.columns.push_back (db_column ("wettbewerbskennzeichen", FIELD_TYPE_BLOB,         0, ""));
		r.columns.push_back (db_column ("bemerkung",              FIELD_TYPE_BLOB,         0, ""));
		r.primary_key="id";
		r.unique_keys.push_back ("id");
		r.unique_keys.push_back ("kennzeichen");
	}
	else if (table_name=="person")
	{
		//                              name,         type,            length, default
		r.columns.push_back (id_column);
		r.columns.push_back (db_column ("nachname",   FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (db_column ("vorname",    FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (db_column ("verein",     FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (db_column ("bwlv",       FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (db_column ("spitzname",  FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (db_column ("vereins_id", FIELD_TYPE_BLOB, 0,      ""));
		r.columns.push_back (db_column ("bemerkung",  FIELD_TYPE_BLOB, 0,      ""));

		r.primary_key="id";
		r.unique_keys.push_back ("id");
	}
	else if (table_name=="user")
	{
		// TODO: the TEXT (BLOB) column can have length "0" (meaning
		// "variable"), but it cannot be used as a key without specifying the
		// key length. db_column does not support specifying a key length, so
		// the field length must be used. Change this.
		//                              name,                  type,            length, default .flags
		r.columns.push_back (db_column ("username",            FIELD_TYPE_VAR_STRING,  255,   "" ).set_not_null (true));
		r.columns.push_back (db_column ("password",            FIELD_TYPE_BLOB,        0,     "" ));
		r.columns.push_back (db_column ("perm_club_admin",     FIELD_TYPE_TINY,        1,     "0"));	// bool is not supported by MySQL
		r.columns.push_back (db_column ("perm_read_flight_db", FIELD_TYPE_TINY,        1,     "0"));	// bool is not supported by MySQL
		r.columns.push_back (db_column ("club",                FIELD_TYPE_BLOB,        0,     ""));
		r.columns.push_back (db_column ("person",              FIELD_TYPE_LONGLONG,    0,     "0"));

		r.primary_key="username";
		r.unique_keys.push_back ("username");
	}

	if (resolve_like && !r.like_table.isEmpty ())
		r=get_table_information (r.like_table, true);
	r.name=table_name;

	return r;
}

void sk_db::check_usability ()
	throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_not_connected, ex_query_failed, ex_unusable, ex_insufficient_access)
{
	// The numbers are references to the corresponding
	// admin_functions.initialize_database section.
	// TODO this has some non-negligible code duplication with admin_functions.initialize_database

	// (0) Connection
	// We must already be connected
	if (!connected ()) throw ex_not_connected ();

	// (2) Check if we can use the database.
	use_db ();

	// (3) Check if all tables exist
	// Build a list of tables that are required.
	QStringList tables_required;
	list_required_tables (tables_required);
	QStringList::const_iterator required_end=tables_required.end ();
	QStringList::const_iterator required_begin=tables_required.begin ();

	// Get a list of tables that are present.
	QStringList tables_present;
	if (list_tables (tables_present)!=db_ok) throw ex_query_failed (last_query);
	QStringList::const_iterator present_end=tables_present.end ();
	QStringList::const_iterator present_begin=tables_present.begin ();

	// Check for the tables
	for (QStringList::const_iterator it=required_begin; it!=required_end; ++it)
		if (std::find (present_begin, present_end, *it)==present_end)
			throw ex_table_not_found (*it);

	// TODO: check writeable

	// (5) Check column types
	// TODO optional columns.

	// We can use tables_required/_end from above.
	for (QStringList::const_iterator required_table_name=required_begin; required_table_name!=required_end; ++required_table_name)
	{
		// required_table_name: iterator to the name of the table we're checking

		// columns_present/_begin/_end: the columns that we have
		QList<db_column> columns_present;
		if (list_columns (columns_present, *required_table_name)!=db_ok) throw ex_query_failed (last_query);
		QList<db_column>::const_iterator columns_present_begin=columns_present.begin ();
		QList<db_column>::const_iterator columns_present_end=columns_present.end ();

		// columns_required/_end: the columns that we need
		db_table required_table=get_table_information (*required_table_name, true);
		QList<db_column> &columns_required=required_table.columns;
		QList<db_column>::const_iterator columns_required_begin=columns_required.begin ();
		QList<db_column>::const_iterator columns_required_end=columns_required.end ();

		// Iterate over the required columns, checking existance and correct type.
		for (QList<db_column>::const_iterator required_column=columns_required_begin; required_column!=columns_required_end; required_column++)
		{
			// required_column: iterator to the column we're checking

			// In the list of columns that we have, find the one with the same name.
			QList<db_column>::const_iterator col;
			for (col=columns_present_begin; col!=columns_present_end; ++col)
				if ((*col).name==(*required_column).name)
					break;

			// col: iterator to the actually existing column with that name, or
			// columns_present_end if not found.

			// Check column existene and type
			if (col==columns_present_end)
				throw ex_column_not_found (*required_table_name, (*required_column).name);
			else if ((*col).type!=(*required_column).type || (*col).length<(*required_column).length)
				throw ex_column_type_mismatch (*required_table_name, (*required_column).name);
		}
	}
}



// Database management
int sk_db::grant (QString permissions, QString target, QString user, QString password)
{
	QString query="GRANT "+permissions+" ON "+target+" TO "+user;
	if (!password.isEmpty ())
	{
		set_query_display_alias (query+" IDENTIFIED BY '***'");
		query=query+" IDENTIFIED BY '"+password+"'";
	}
	return execute_query (query);
}

int sk_db::grant (QString permissions, QString target, QString user)
{
	return grant (permissions, target, user, "");
}

int sk_db::create_database (QString name, bool force)
{
	QString query="CREATE DATABASE ";
	if (!force) query+="IF NOT EXISTS ";
	query+=name;

	return execute_query (query);
}

int sk_db::create_table (const db_table &tab, bool force)
{
	QString query=tab.mysql_create_query (force);
	return execute_query (query);
}

int sk_db::list_tables (QStringList &tables)
{
	return list_strings_query ("SHOW TABLES", 0, tables);
}

int sk_db::list_column_names (QStringList &names, QString table)
{
	QList<db_column> columns;
	int ret=list_columns (columns, table);

	QList<db_column>::const_iterator end=columns.end ();
	for (QList<db_column>::const_iterator it=columns.begin (); it!=end; ++it)
		names.push_back ((*it).name);

	return ret;
}

int sk_db::list_columns (QList<db_column> &columns, QString table)
{
	MYSQL_RES *result;

	// SELECT * FROM table WHERE false
	// Now, this is a bit of a hack. Any other way to do this?
	// mysql_list_fields, maybe? Note that the documentation lists this
	// function as deprecated (?).
	int ret=execute_query (&result, "SELECT * FROM "+table+" WHERE false", true);

	if (ret==db_ok)
	{
		unsigned int num_fields=mysql_num_fields (result);
		MYSQL_FIELD *fields=mysql_fetch_fields (result);
		for (unsigned int i=0; i<num_fields; i++)
		{
			db_column c (fields[i]);
			columns.push_back (c);
		}

		mysql_free_result (result);

		return db_ok;
	}

	return ret;
}

int sk_db::add_column (const QString &table, const db_column &column)
{
	QString query="ALTER TABLE "+table+" ADD COLUMN "+column.mysql_spec ();
	return execute_query (query);
}

int sk_db::modify_column (const QString &table, const db_column &column)
{
	QString query="ALTER TABLE "+table+" MODIFY COLUMN "+column.mysql_spec ();
	return execute_query (query);
}


// Generic functions
char *sk_db::named_field_value (MYSQL_ROW &row, int num_fields, MYSQL_FIELD *fields, const char *name)
	// This function probably is quite slow, especially when called often,
	// there should be a better way to do this.
{
	// Determine the field number by name
	int field_num=-1;
	for (int i=0; i<num_fields; i++)
	{
		if (fields && name && fields[i].name && strcmp (name, fields[i].name)==0)
		{
			field_num=i;
			break;
		}
	}

	if (field_num<0)
	{
		log_error (QString ("Field ")+QString (name)+QString (" not found in sk_db::named_field_value ()"));
		return (char *)"";
	}
	else
	{
		return row[field_num]?row[field_num]:(char *)"";
	}
}

const char *sk_db::field_value (const MYSQL_ROW &row, const unsigned int num_fields, MYSQL_FIELD *fields, const unsigned int num)
{
	if (num>=num_fields)
	{
		log_error ("Field number "+QString::number (num)+" out of range in sk_db::named_field_value ()");
		return "";
	}

	return row[num]?row[num]:(char *)"";
}

QString sk_db::escape (QString text)
{
	if (connected ())
	{
		QByteArray i=text.toAscii ();
		const char *input=text.latin1();
		unsigned long len=text.length ();
		char *result_buffer=(char *)malloc (2*len+1);
		mysql_real_escape_string (mysql, result_buffer, input, len);
		QString result (result_buffer);
		free (result_buffer);
		return result;
	}
	else
	{
		return "";
	}
}


// Database metainformation (table names etc.)
QString sk_db::fixed_table_name (db_object_type type)
{
	switch (type)
	{
		case ot_flight: return "flug"; break;
		case ot_plane: return "flugzeug"; break;
		case ot_person: return "person"; break;
		default: return "unknown"; break;	// TODO Fehlermeldung
	}
}

QString sk_db::editable_table_name (db_object_type type)
{
	switch (type)
	{
		case ot_flight: return "flug_temp"; break;
		case ot_plane: return "flugzeug_temp"; break;
		case ot_person: return "person_temp"; break;
		default: return "unknown"; break;	// TODO Fehlermeldung
	}
}

QString sk_db::default_sort_column (db_object_type type)
{
	switch (type)
	{
		case ot_flight: return "";	// TODO effdate, aber woher nehmen?
		case ot_plane: return "kennzeichen";
		case ot_person: return "nachname,vorname,verein";
		default:
			debug_stream << "[db] Error: unhandled object type in sk_db::default_sort_column" << std::endl;
			return "";
			break;
	}
}


// Data structures
QString sk_db::query_column_list (db_object_type otype, bool id_only)
	// TODO id_only weg und daf�r �berall column_name_id verwenden.
{
	if (id_only)
	{
		return "id";
	}
	else
	{
		switch (otype)
		{
			case ot_flight:
				if (opts.record_towpilot)
					return "id,pilot,begleiter,towpilot,startort,zielort,anzahl_landungen,startzeit,landezeit,startart,land_schlepp,typ,bemerkung,flugzeug,status,modus,pvn,pnn,bvn,bnn,tpvn,tpnn,modus_sfz,zielort_sfz,abrechnungshinweis,towplane";
				else
					return "id,pilot,begleiter,startort,zielort,anzahl_landungen,startzeit,landezeit,startart,land_schlepp,typ,bemerkung,flugzeug,status,modus,pvn,pnn,bvn,bnn,modus_sfz,zielort_sfz,abrechnungshinweis,towplane";
				break;
			case ot_plane:
				return "id,kennzeichen,verein,sitze,typ,gattung,wettbewerbskennzeichen,bemerkung";
				break;
			case ot_person:
				return "id,nachname,vorname,verein,vereins_id,bwlv,bemerkung";
				break;
			case ot_none:
				return "*";
				break;
			default:
				debug_stream << "Program error: Unhandled object type in sk_db::query_column_list ()" << std::endl;
				break;
		}
	}

	return "";
}

QString sk_db::query_value_list (db_object_type type, void *object)
{
	// TODO unify with query_column_list and row_to_object
	std::ostringstream oss;
	QString r;

	// Use the OUTPUT macro for all values but the last and use OUTPUTL there.
	// Key needs to be anything that can be +ed to a QString (probably a
	// QString or a char *), value needs to be anything that can be <<ed to
	// an output stream (basically strings and numbers).
#define OUTPUTL(key, value) r+=key; r+="='"; oss.str (""); oss << value; r+=escape (std2q (oss.str ())); r+="'";
#define OUTPUT(key, value) OUTPUTL (key, value) r+=", ";

	// Note: do not output the ID here. It is not explicitly written.
	switch (type)
	{
		case ot_flight:
		{
			Flight *flight=(Flight *)object;
			OUTPUT ("pilot", flight->pilot);
			OUTPUT ("begleiter", flight->begleiter);
			OUTPUT ("startort", flight->startort);
			OUTPUT ("zielort", flight->zielort);
			OUTPUT ("anzahl_landungen", flight->landungen);
			OUTPUT ("startzeit", to_string (&(flight->startzeit)));
			OUTPUT ("landezeit", to_string (&(flight->landezeit)));
			OUTPUT ("startart", flight->startart);
			OUTPUT ("land_schlepp", to_string (&(flight->landezeit_schleppflugzeug)));
			OUTPUT ("typ", flugtyp_to_db (flight->flugtyp));
			OUTPUT ("bemerkung", flight->bemerkungen);
			OUTPUT ("flugzeug", flight->flugzeug);
			OUTPUT ("status", make_status (flight->gestartet, flight->gelandet, flight->sfz_gelandet));
			OUTPUT ("modus", modus_to_db (flight->modus));
			OUTPUT ("pvn", flight->pvn);
			OUTPUT ("pnn", flight->pnn);
			OUTPUT ("bvn", flight->bvn);
			OUTPUT ("bnn", flight->bnn);
			if (opts.record_towpilot)
			{
				OUTPUT ("towpilot", flight->towpilot);
				OUTPUT ("tpvn", flight->tpvn);
				OUTPUT ("tpnn", flight->tpnn);
			}
			OUTPUT ("modus_sfz", modus_to_db (flight->modus_sfz));
			OUTPUT ("zielort_sfz", flight->zielort_sfz);
			OUTPUT ("towplane", flight->towplane);
			OUTPUTL ("abrechnungshinweis", flight->abrechnungshinweis);
		} break;
		case ot_plane:
		{
			Plane *plane=(Plane *)object;
			OUTPUT ("kennzeichen", plane->registration);
			OUTPUT ("verein", plane->club);
			OUTPUT ("sitze", plane->sitze);
			OUTPUT ("typ", plane->typ);
			OUTPUT ("gattung", category_to_db (plane->category));
			OUTPUT ("wettbewerbskennzeichen", plane->wettbewerbskennzeichen);
			OUTPUTL ("bemerkung", plane->bemerkungen);
		} break;
		case ot_person:
		{
			Person *person=(Person *)object;
			OUTPUT ("nachname", person->nachname);
			OUTPUT ("vorname", person->vorname);
			OUTPUT ("vereins_id", person->club_id);
			OUTPUT ("verein", person->club);
			OUTPUT ("bwlv", person->landesverbands_nummer);
			OUTPUTL ("bemerkung", person->bemerkungen);
		} break;
		default:
			debug_stream << "Program error: Unhandled object type in sk_db::query_column_list ()" << std::endl;
			break;
	}

#undef OUTPUT
#undef OUTPUTL

	return r;
}

int sk_db::row_to_object (db_object_type otype, void *object, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields)
{
	// Key is a char *. This macro sets the variable value to the value.
#define USE(key) value=named_field_value (row, num_fields, fields, key);
	char *value;

	switch (otype)
	{
		case ot_flight:
		{
			Flight *p=(Flight *)object;
			// TODO: hier pr�fen, ob diese Felder existieren.
			USE ("id");                 p->id=atoll (value);
			USE ("pilot");              p->pilot=atol (value);
			USE ("begleiter");          p->begleiter=atol (value);
			USE ("startort");           p->startort=(QString)value;
			USE ("zielort");            p->zielort=(QString)value;
			USE ("anzahl_landungen");   p->landungen=atoi (value);
			USE ("startzeit");          parse (&(p->startzeit), value);
			USE ("landezeit");          parse (&(p->landezeit), value);
			USE ("startart");           p->startart=atol (value);
			USE ("land_schlepp");       parse (&(p->landezeit_schleppflugzeug), value);
			USE ("typ");                p->flugtyp=db_to_flugtyp (value);
			USE ("bemerkung");          p->bemerkungen=(QString)value;
			USE ("flugzeug");           p->flugzeug=atol (value);
			USE ("status"); unsigned int status=atoi (value);
			p->gestartet=status_gestartet (status);
			p->gelandet=status_gelandet (status);
			p->sfz_gelandet=status_sfz_gelandet (status);
			USE ("modus");              p->modus=db_to_modus (value);
			USE ("pvn");                p->pvn=(QString)value;
			USE ("pnn");                p->pnn=(QString)value;
			USE ("bvn");                p->bvn=(QString)value;
			USE ("bnn");                p->bnn=(QString)value;
			if (opts.record_towpilot)
			{
				USE ("towpilot");            p->towpilot=atol (value);
				USE ("tpvn");                p->tpvn=(QString)value;
				USE ("tpnn");                p->tpnn=(QString)value;
			}
			USE ("modus_sfz");          p->modus_sfz=db_to_modus (value);
			USE ("zielort_sfz");        p->zielort_sfz=(QString)(value);
			USE ("towplane"); 			p->towplane=atoi(value);
			USE ("abrechnungshinweis"); p->abrechnungshinweis=(QString)(value);
			USE (column_name_editable.latin1()); p->editierbar=(atoi (value)>0);
		} break;
		case ot_plane:
		{
			Plane *p=(Plane *)object;
			USE ("id");						p->id=atoll (value);
			USE ("kennzeichen");			p->registration=value;
			USE ("verein");					p->club=value;
			USE ("sitze");					p->sitze=atoi (value);
			USE ("typ");					p->typ=value;
			USE ("gattung");				p->category=db_to_category (value);
			USE ("wettbewerbskennzeichen");	p->wettbewerbskennzeichen=value;
			USE ("bemerkung");				p->bemerkungen=value;
			USE (column_name_editable.latin1()); p->editierbar=(atoi (value)>0);
		} break;
		case ot_person:
		{
			Person *p=(Person *)object;
			USE ("id");				p->id=atoll (value);
			USE ("nachname");		p->nachname=value;
			USE ("vorname");		p->vorname=value;
			USE ("verein");			p->club=value;
			USE ("vereins_id");		p->club_id=value;
			USE ("bwlv");			p->landesverbands_nummer=value;
			USE ("bemerkung");		p->bemerkungen=value;
			USE (column_name_editable.latin1()); p->editierbar=(atoi (value)>0);
		} break;
		case ot_none:
			break;
		default:
			debug_stream << "Program error: Unhandled object type in sk_db::row_to_object ()" << std::endl;
			return db_err_unhandled;
			break;
	}
#undef USE

	return db_ok;
}

long long int sk_db::row_to_number (MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields, const char *field_name)
{
	return atoll (named_field_value (row, num_fields, fields, field_name));
}

QString sk_db::to_string (sk_time_t *t)
{
	if (t->is_null ())
		return db_null_time;
	else
		return t->to_string (db_time_format, tz_utc, 20, true);
}

void sk_db::parse (sk_time_t *time, QString text)
{
	if (text==db_null_time)
		time->set_null ();
	else
		time->parse (text, tz_utc, db_time_format);
}

int sk_db::copy_object (db_object_type type, void *target, void *source)
{
	if (!source) return db_err_parameter_error;
	if (!target) return db_err_parameter_error;

#define CASE(object_type, data_type) case object_type: (*(data_type *)target)=(*(data_type *)source); return db_ok; break;
	switch (type)
	{
		CASE (ot_flight, Flight)
		CASE (ot_plane, Plane)
		CASE (ot_person, Person)
		// No default to enable compiler warnings (g++ -Wall)
		case ot_none: debug_stream << "[db] Error: object type ot_none in copy_object ()" << std::endl; return db_err_parameter_error; break;
	}
#undef CASE

	debug_stream << "[db] Error: unhandled object type in sk_db::copy_object" << std::endl;
	return db_err_unhandled;
}

void *sk_db::new_object (db_object_type type)
{
	switch (type)
	{
		case ot_flight: return new Flight; break;
		case ot_plane: return new Plane; break;
		case ot_person: return new Person; break;
		case ot_none: debug_stream << "[db] Error: object type ot_none in new_object ()" << std::endl; return NULL;
	}

	debug_stream << "[db] Error: unhandled object type in sk_db::new_object ()" << std::endl;
	return NULL;
}

int sk_db::free_object (db_object_type type, void *object)
{
	switch (type)
	{
		case ot_flight: delete (Flight *)object; return db_ok; break;
		case ot_plane: delete (Plane *)object; return db_ok; break;
		case ot_person: delete (Person *)object; return db_ok; break;
		case ot_none: debug_stream << "[db] Error: object type ot_none in free_object ()" << std::endl; return db_err_parameter_error; break;
	}

	debug_stream << "[db] Error: unhandled object type in sk_db::free_object ()" << std::endl;
	return db_err_unhandled;
}

QString sk_db::object_name (db_object_type type, bool plural)
{
	switch (type)
	{
		case ot_flight: return plural?"flights":"flight"; break;
		case ot_plane: return plural?"plane":"planes"; break;
		case ot_person: return plural?"person":"persons"; break;
		case ot_none: return plural?"none objects":"none object"; break;
	}

	debug_stream << "[db] Error: unhandled object type in sk_db::free_object ()" << std::endl;
	return plural?"unknwon objects":"unknwon object";
}



// Enumeration types
//QString sk_db::startart_to_db (flug_startart s)
//{
//	// TODO Nicht hardcoden sondern zumindest in die
//	// Konfigurationsdatei, besser in die Datenbank
//
//	switch (s)
//	{
//		case sa_wa: return "1"; break;
//		case sa_wk: return "2"; break;
//		case sa_we: return "3"; break;
//		case sa_av: return "4"; break;
//		case sa_gz: return "5"; break;
//		case sa_ss: return "7"; break;
//		case sa_sonstige: return "8"; break;
//		case sa_keine: return "9"; break;
//		default:
//			log_error ("Unbekannte Startart in sk_db::startart_to_db ()");
//			return "-";
//			break;
//	}
//
//	return "-";
//}

QString sk_db::flugtyp_to_db (flug_typ t)
{
	switch (t)
	{
		case ft_normal: return "2"; break;
		case ft_schul_2: return "3"; break;
		case ft_schul_1: return "4"; break;
		case ft_gast_privat: return "6"; break;
		case ft_gast_extern: return "8"; break;
		case ft_schlepp: return "7"; break;
		case ft_kein: return "1"; break;
		default:
			log_error ("Unbekannter Flugtyp in sk_db::flugtyp_to_db ()");
			return "-";
			break;
	}

	return "-";
}

QString sk_db::modus_to_db (flug_modus m)
{
	switch (m)
	{
		case fmod_lokal: return "l"; break;
		case fmod_geht: return "g"; break;
		case fmod_kommt: return "k"; break;
		case fmod_kein: return "-"; break;
		default:
			log_error ("Unbekannter Flugmodus in modus_to_db ()");
			return "-";
			break;
	}

	return "-";
}

QString sk_db::category_to_db (aircraft_category g)
{
	switch (g)
	{
		case lfz_echo: return "e"; break;
		case lfz_segelflugzeug: return "1"; break;
		case lfz_motorsegler: return "k"; break;
		case lfz_ultraleicht: return "m"; break;
		case lfz_sonstige: return "s"; break;
		case lfz_keine: return "-"; break;
		default:
			log_error ("Unknown category in category_to_db ()");
			return "-";
			break;
	}

	return "-";
}

flug_typ sk_db::db_to_flugtyp (char *in)
{
	switch (in[0])
	{
		case '1': return ft_kein;
		case '2': return ft_normal;
		case '3': return ft_schul_2;
		case '4': return ft_schul_1;
		case '5': std::cerr << "In der Datenbank steht ein Kunstflug" << std::endl; return ft_normal;
		case '6': return ft_gast_privat;
		case '8': return ft_gast_extern;
		case '7': return ft_schlepp;
		default:
			std::cerr << "Unknown Flight type " << in[0] << " in db_to_flugtyp ()" << std::endl;
			return ft_kein;
	}
}

flug_modus sk_db::db_to_modus (char *in)
{
	switch (in[0])
	{
		case 'l': return fmod_lokal; break;
		case 'k': return fmod_kommt; break;
		case 'g': return fmod_geht; break;
		default:
			std::cerr << "Unknown flight mode '" << in << "' in db_to_modus ()" << std::endl;
			return fmod_kein;
			break;
	}

	return fmod_kein;
}

aircraft_category sk_db::db_to_category (char *in)
{
	switch (in[0])
	{
		case 'e': return lfz_echo; break;
		case '1': return lfz_segelflugzeug; break;
		case 'k': return lfz_motorsegler; break;
		case 'm': return lfz_ultraleicht; break;
		case 's': return lfz_sonstige; break;
		default:
			std::cerr << "Unknown category in db_to_category ()" << std::endl;
			return lfz_keine;
			break;
	}

	return lfz_keine;
}


// Bitmask types
bool sk_db::status_gestartet (unsigned int status)
{
	return (status & STATUS_GESTARTET);
}

unsigned int sk_db::make_status (bool gestartet, bool gelandet, bool sfz_gelandet)
{
	int r=0;
	if (gestartet) r|=STATUS_GESTARTET;
	if (gelandet) r|=STATUS_GELANDET;
	if (sfz_gelandet) r|=STATUS_SFZ_GELANDET;
	return r;
}

bool sk_db::status_gelandet (unsigned int status)
{
	return (status & STATUS_GELANDET);
}

bool sk_db::status_sfz_gelandet (unsigned int status)
{
	return (status & STATUS_SFZ_GELANDET);
}


// Querying core functions
int sk_db::execute_query (MYSQL_RES **result, QString query_string, bool retrieve_immediately)
	// The result is valid exactly if 0 is returned.
	// You must call mysql_free_result on the result if it is valid.
	// result may be NULL in which case it freed by this function.
{
	if (!connected ()) return db_err_not_connected;

	QString display_query;
	if (query_display_alias.isEmpty ())
	{
		display_query=query_string;
	}
	else
	{
		display_query=query_display_alias;
		query_display_alias="";
	}

	if (display_queries)
		debug_stream << display_query << std::endl;

	emit executing_query (&display_query);
	last_query=display_query;

	MYSQL_RES *res;

	if (mysql_real_query (mysql, query_string.latin1(), query_string.length ())==0)
	{
		// The query succeeded.
		last_query_success=true;

		if (retrieve_immediately)
			// Retrieve the data from the server
			res=mysql_store_result (mysql);
		else
			// Prepare the data for retrieval from the server.
			res=mysql_use_result (mysql);

		// It is legal for res to be NULL, for example, when the query
		// returns no data.
		if (!res && mysql_errno (mysql)!=0)
		{
			debug_stream << "[db] " << (retrieve_immediately?"mysql_store_result":"mysql_use_result") << " returned an error in sk_db::execute_query (): " << get_last_error () << std::endl;
			return db_err_mysql_error;
		}

		if (result)
			*result=res;
		else
			if (res) mysql_free_result (res);

		return db_ok;
	}
	else
	{
		// The query failed.
		last_query_success=false;
		debug_stream << "[db] mysql_real_query returned the error '"<< get_last_error () <<"'. The query that failed was:" << std::endl;
		debug_stream << ">>>>>>>>" << std::endl;
		debug_stream << display_query << std::endl;
		debug_stream << "<<<<<<<<" << std::endl;
#ifdef DEBUG
//	SEGFAULT
#endif

		if (result) *result=NULL;
		return db_err_query_failed;
	}
}

QString sk_db::make_query (query_type_t query_type, db_object_type object_type, QString condition, QString columns, bool add_column_editable, bool distinct, QString sort_column, QString group_column, bool editable_flag)
	/*
	 * This function is mainly used for making queries where two
	 * tables (name and name_temp) have to be considered.
	 * Other queries may be constructed directly.
	 * editable_flag is only honored for admin_db.
	 */
{
	// TODO replace "id" by column_name_id
	QString query;
	QString editable_table=editable_table_name (object_type);
	QString fixed_table=fixed_table_name (object_type);

	QString target_table;
	if (is_admin_db && !editable_flag)
		// This is an admin connection and the object is to be created
		// non-editable. Write to the fixed database.
		target_table=fixed_table;
	else
		target_table=editable_table;


	switch (query_type)
	{
		case qt_create:
		{
			// insert into $table set $values
			query="insert into "+target_table+" set "+columns;
		} break;
		case qt_update:
			//// update $editable_table set $values where $condition and $editalbe_table.id not in (select id from $fixed_table)
			//query="update "+editable_table+" set "+columns+" where "+condition+" AND "+editable_table+".id not in (select id from "+fixed_table+")";
			// update $table set $values where $condition
			query="update "+target_table+" set "+columns+" where "+condition;
			break;
		case qt_delete:
		{
			//// delete from $editable_table where $condition and $editalbe_table.id not in (select id from $fixed_table)
			//query="delete from "+editable_table+" where "+condition+" AND "+editable_table+".id not in (select id from "+fixed_table+")";
			// delete from $table where $condition
			query="delete from "+target_table+" where "+condition;
		} break;
		case qt_list:
		{
			// select [distinct] $columns,0 as editable from $fixed_table
			//   where $condition
			// UNION select [distinct] $columns,1 as editable from $editable_table
			//   where $condition
			//   and $editable_table.id not in (select id from $fixed_table)
			//   [order bey $sort_column] [group by $group_column]
			QString editable_condition="("+condition+") AND ("+editable_table+".id not in (select id from "+fixed_table+"))";

			QString column_editable_yes;
			QString column_editable_no;
			if (add_column_editable)
			{
				column_editable_yes=",1 as "+column_name_editable;
				column_editable_no=",0 as "+column_name_editable;
			}

			QString distinct_string="";
			if (distinct) distinct_string="distinct ";


			query="select "+distinct_string+columns+column_editable_no+" from "+fixed_table+" where ("+condition
				+") UNION select "+distinct_string+columns+column_editable_yes+" from "+editable_table+" where "+editable_condition;

			if (!sort_column.isEmpty ()) query+=" order by "+sort_column;
			if (!group_column.isEmpty ()) query+=" group by "+group_column;
		} break;
//		case qt_count:
			/*
			 * Problem here:
			 * I need the number of entries in the fixed table, plus the number
			 * of entries in the editable table whose ID is not in the fixed
			 * table. I could use, for the entries:
			 *   select id from flug union select id from flug_temp
			 * For getting the number, according to web sources, the following
			 * should do:
			 *   select COUNT(*) from ($(the above))
			 * However, this fails with
			 *   ERROR 1246: Every derived table must have it's own alias
			 * Using, instead,
			 *   select COUNT(*) from (($(the above)) as foobar)
			 * fails with
			 *   ERROR 1142: select command denied to user: 'ases@localhost' for table '/tmp/#sql_559_0'
			 * Trying to select the counts individually:
			 *   select COUNT(*) from flug UNION ALL (select COUNT(*) from flug_temp where id not in (select id from flug))
			 *   (IMPORTANT: note the ALL in UNION ALL! Else there would be an
			 *   error if both parts return the same number)
			 * This works fine, but when trying to sum them up:
			 *   select sum(subtotal) as total from ((select COUNT(*) from flug as subtotal) UNION ALL (select COUNT(*) from flug_temp as subtotal where...))
			 * Gives the same 1246 as above.
			 */
//			query="";
//			break;
		case qt_count_murx:
			// (select COUNT(*) as count from $fixed where $condition) UNION ALL (select COUNT(*) as count from $editable where ($condition and id not in (select id from $fixed)))
			query="(select COUNT(*) as count from "+fixed_table+" where ("+condition+")) "
				+"UNION ALL (select COUNT(*) as count from "
				+editable_table+" where (("+condition+") and (id not in (select id from "+fixed_table+"))))";
			break;
//		case qt_describe:
//			// describe $table
//			query="describe ";
//			break;
		default:
			debug_stream << "[db] Unhandled query_type in sk_db::make_query ()" << std::endl;
			query="";
			break;
	}

	return query;
}

QString sk_db::make_condition (condition_t c)
	// TODO: move this to contition_t::? Note that this calls make_query.
	// Maybe make a class query_t?
	// The fourth attempt to write a condition making function.
	// We pass (mostly unused) typed parameters and use the preprocessor to
	// construct the strings (for runtime efficiency). We also have to add a
	// separate contition_type_t value for each condition we want to use.
	// Conditions can be ANDed externally using the and_condition function.
	// If we had named parameters, we could use recursion nicely here.
	// Note the new definition of "currently flying", meaning "is still to
	// land". This leads to the possibility of there being more flying than
	// total flights when having prepared coming flights.
	// However, we provide a wrapper for each combination used because this
	// function is not public (and should not be).
{
	// Note the convention for parentheses placement is "a=(x+y+z)", not
	// "a=(x)+(y)+(z)"! (Rationale: Here we know if we need parentheses, for
	// example, FUNC (foobar) does not need any, neither does TRUE.)
#define AND +QString (" AND ")+
#define OR +QString (" OR ")+
#define START QString ("(")+
#define END +QString (")")
#define EQ +QString ("=")+
#define QUOTED(a) "'"+a+"'"
#define EQUALS(key, value) START #key EQ "'"+value+"'" END
#define IF(a,b,c) "IF ("+a+","+b+","+c+")"
#define NOT(a) START "!"+a END

	// Parameter to QString conversion
	// Escaping is done here to ensure each parameter is escaped exactly once
#define param_id			escape (QString::number (c.id1))
#define param_date1			escape ((c.date1?(c.date1->toString (Qt::ISODate)):QString ("")))
#define param_date2			escape ((c.date2?(c.date2->toString (Qt::ISODate)):QString ("")))
#define param_given_time	escape ((c.given_time1?to_string (c.given_time1):QString ("")))
#define param_text1			escape (c.text1?*(c.text1):QString (""))
#define param_text2			escape (c.text2?*(c.text2):QString (""))
#define param_date			param_date1

	// Generic conditions
#define _id EQUALS (id, param_id)
#define _any "true"
#define _none "false"

	// Flight conditions
#define _flight_pilot EQUALS (pilot, param_id)
#define _flight_person START EQUALS (pilot, param_id) OR EQUALS (begleiter, param_id) END
#define _flight_plane EQUALS (flugzeug, param_id)
#define _flight_plane_or_towplane START EQUALS (flugzeug, param_id) OR EQUALS (towplane, param_id) END
#define _flight_mode_local EQUALS (modus, modus_to_db (fmod_lokal))
#define _flight_mode_coming EQUALS (modus, modus_to_db (fmod_kommt))
#define _flight_mode_going EQUALS (modus, modus_to_db (fmod_geht))
#define _flight_starts_here START _flight_mode_local OR _flight_mode_going END
#define _flight_started START "status & "+QString::number (STATUS_GESTARTET) END
#define _flight_landed START "status & "+QString::number (STATUS_GELANDET) END
#define _flight_towplane_landed START "status & "+QString::number (STATUS_SFZ_GELANDET) END
		// Handle flights of which the starting time is not known.
#define _flight_landed_only START NOT (_flight_started) AND _flight_landed END
#define _flight_local_landed_only START _flight_mode_local AND _flight_landed_only END
//		// TODO: This assumes that every flight at least lands or starts here.
#define _flight_happened START IF (_flight_starts_here, _flight_started, _flight_landed) OR _flight_local_landed_only END
#define _flight_prepared NOT (_flight_happened)
#define expr_flight_effdate IF (_flight_starts_here AND NOT (_flight_landed_only), "startzeit", "landezeit")
#define _flight_happened_on_date START _flight_happened AND START "DATE_FORMAT("+expr_flight_effdate+", '%Y-%m-%d')" EQ QUOTED (param_date) END END
#define _flight_after_date START _flight_happened AND START expr_flight_effdate+">="+QUOTED (param_date+" 00:00:00") END END
#define _flight_before_date START _flight_happened AND START expr_flight_effdate"<="+QUOTED (param_date+" 23:59:59") END END
	// TODO code duplication
#define _flight_before_date2 START _flight_happened AND START expr_flight_effdate"<="+QUOTED (param_date2+" 23:59:59") END END
	// TODO this query contains _flight_happened twice
#define _flight_happened_between START _flight_after_date AND _flight_before_date2 END
#define _flight_started_before START _flight_started AND START "startzeit<="+QUOTED (param_given_time) END END
#define _flight_landed_after START _flight_landed AND START "landezeit>="+QUOTED (param_given_time) END END
	// TODO: This condition seems unnecessarily complicated
#define _flight_start_date_like START "DATE_FORMAT(startzeit, '%Y-%m-%d')=DATE_FORMAT("+QUOTED (param_given_time)+", "+QUOTED ("%Y-%m-%d")+")" END
#define _flight_flying_at START _flight_mode_local AND _flight_start_date_like AND _flight_started_before AND START NOT (_flight_landed) OR _flight_landed_after END END
#define _flight_local_will_land START _flight_mode_local AND _flight_happened_on_date AND NOT (_flight_landed) END
#define _flight_coming_will_land START _flight_mode_coming AND NOT (_flight_landed) END
#define _flight_will_land START _flight_local_will_land OR _flight_coming_will_land END
#define _flight_plane_flying_at START _flight_plane AND _flight_flying_at END
#define _flight_person_flying_at START _flight_person AND _flight_flying_at END
#define _flight_plane_date START _flight_plane AND _flight_happened_on_date END
#define _flight_person_date START _flight_person AND _flight_happened_on_date END
#define _flight_person_date_range START _flight_person AND _flight_happened_between END

	// Person conditions
#define _person_first_name EQUALS (vorname, param_text1)
#define _person_last_name EQUALS (nachname, param_text1)
#define _person_name START EQUALS (vorname, param_text1) AND EQUALS (nachname, param_text2) END
#define _person_club_club_id START EQUALS (verein, param_text1) AND EQUALS (vereins_id, param_text2) END

	// Plane conditions
#define _plane_registration EQUALS (kennzeichen, param_text1)

	QString result;

#define CASE(cname) case cond_ ## cname: result= _ ## cname; break;
	switch (c.type)
	{
		// Generic conditions
		CASE (id)
		CASE (any)
		CASE (none)
		// Flight conditions
		CASE (flight_pilot)
		CASE (flight_person)
		CASE (flight_plane)
		CASE (flight_plane_or_towplane)
		CASE (flight_mode_local)
		CASE (flight_mode_coming)
		CASE (flight_mode_going)
		CASE (flight_starts_here)
		CASE (flight_started)
		CASE (flight_landed)
		CASE (flight_towplane_landed)
		CASE (flight_happened)
		CASE (flight_prepared)
		CASE (flight_happened_on_date)
		CASE (flight_after_date)
		CASE (flight_before_date)
		CASE (flight_started_before)
		CASE (flight_happened_between)
		CASE (flight_landed_after)
		CASE (flight_start_date_like)
		CASE (flight_flying_at)
		CASE (flight_will_land)
		CASE (flight_plane_flying_at)
		CASE (flight_person_flying_at)
		CASE (flight_plane_date)
		CASE (flight_person_date)
		CASE (flight_person_date_range)
		// Person conditions
		CASE (person_first_name)
		CASE (person_last_name)
		CASE (person_name)
		CASE (person_club_club_id)
		// Plane conditions
		CASE (plane_registration)
		// No default label to allow compiler warning (g++ -Wall)
	}

	// If the condition is empty, that means we have an unhandled case above.
	if (result.isEmpty ())
	{
		debug_stream << "[db] unhandled condition type in sk_db::condition_part ()" << std::endl;
		result="false";
	}

	if (c.meta_query_table!=ot_none)
	{
		// Meta query. Now we have "condition". What we want is "id in
		// (select id from flug_temp where condition UNION select id from flug
		// where...)". We construct this by a call to make_query.

		QString meta_query_column=c.meta_query_column?*(c.meta_query_column):"";
		QString meta_query=make_query (qt_list, c.meta_query_table, result, meta_query_column, false, false);
		result=column_name_id+" in ("+meta_query+")";
	}

	return result;
#undef AND
#undef OR
#undef START
#undef END
#undef EQ
#undef QUOTED
#undef EQUALS
#undef IF
#undef NOT
#undef param_id
#undef param_date
#undef param_given_time
#undef param_text1
#undef _id
#undef _any
#undef _none
#undef _flight_pilot
#undef _flight_person
#undef _flight_plane
#undef _flight_mode_local
#undef _flight_mode_coming
#undef _flight_mode_going
#undef _flight_starts_here
#undef _flight_started
#undef _flight_landed
#undef _flight_towplane_landed
#undef _flight_happened
#undef _flight_prepared
#undef expr_flight_effdate
#undef _flight_happened_on_date
#undef _flight_after_date
#undef _flight_before_date
#undef _flight_started_before
#undef _flight_before_date2
#undef _flight_happened_between
#undef _flight_landed_after
#undef _flight_start_date_like
#undef _flight_flying_at
#undef _flight_local_will_land
#undef _flight_coming_will_land
#undef _flight_will_land
#undef _flight_plane_flying_at
#undef _flight_person_flying_at
#undef _flight_plane_date
#undef _flight_person_date
#undef _person_first_name
#undef _person_last_name
#undef _person_name
#undef _plane_registration
#undef CASE
}


// Direct access helper functions
bool sk_db::object_exists_in (const QString &table, const db_id &id)
	throw (ex_legacy_error)
	/*
	 * Checks if an object with at given ID exists a a given table.
	 * Parameters:
	 *   - table: the table too search in
	 *   - id: the ID to search for
	 * Return value:
	 *   - true if id exists in table.
	 */
{
	const QString condition=make_condition (condition_t (cond_id, id));
	int ret=rows_exist_query ("select "+column_name_id+" from "+table+" where "+condition);
	if (ret<0) throw ex_legacy_error (ret, *this);
	if (ret>0) return true;
	return false;
}

void sk_db::object_delete_from (const QString &table, const db_id &id)
	throw (ex_legacy_error)
	/*
	 * Deletes an object from a table.
	 * No checking if the object exists is done. No success checking is done.
	 * Paramters:
	 *   - table: the name of the table to delete from.
	 *   - id: the ID of the object to delete.
	 */
{
	const QString condition=make_condition (condition_t (cond_id, id));
	int ret=execute_query ("delete from "+table+" where "+condition);
	if (ret<0) throw ex_legacy_error (ret, *this);
}

void sk_db::object_copy (const QString &source, const QString &target, db_id id)
	throw (ex_legacy_error)
	/*
	 * Copies an object from one table to another.
	 * No checking if the object exists is done. The object must not exist in
	 * the target table.
	 * Parameters:
	 *   - source: the name of the table to copy from.
	 *   - target: the name of the table to copy to. The structure of the
	 *     target table must match the structure of the source table.
	 *   - id: the ID of the object to copy.
	 */
{
	const QString condition=make_condition (condition_t (cond_id, id));
	const QString query="insert into "+target+" select * from "+source+" where "+condition;
	int ret=execute_query (query);
	if (ret<0) throw ex_legacy_error (ret, *this);
}


// Editable
void sk_db::make_editable (db_object_type type, int id, bool editable)
	throw (ex_legacy_error, ex_operation_failed)
	/*
	 * Changes the state of an object from editabe to non-editable or vice versa.
	 * Paramters:
	 *   - type: the object type. This determines the tables used.
	 *   - id: thd ID of the object to change.
	 *   - editable: whether the object should be made editable or non-editable.
	 */
{
	if (id_invalid (id)) throw ex_legacy_error (db_err_parameter_error, *this);
	if (!is_admin_db) throw ex_legacy_error (db_err_not_admin_db, *this);

	// The names of the tables.
	QString editable_table=editable_table_name (type);
	QString fixed_table=fixed_table_name (type);

	// Whether the object exists in the tables.
	bool exists_editable=object_exists_in (editable_table, id);
	bool exists_fixed=object_exists_in (fixed_table, id);

	// If the object does not exist at all, this is an error.
	if (!exists_fixed && !exists_editable) throw ex_legacy_error (db_err_not_found, *this);

	if (editable)
	{
		// Make editable
		if (exists_fixed)
		{
			// The object exists in the fixed table.
			// Copy it to the editable table and, if successful, delete it from
			// the fixed table.
			// We need to delete it from the target table first.

			// Delete the object from the editable table, if it exists.
			if (exists_editable) object_delete_from (editable_table, id);
			exists_editable=object_exists_in (editable_table, id);

			// If the object still exists in the editable table, this is an error.
			if (exists_editable) throw ex_operation_failed ("Objekt existiert nach L�schen aus editierbarer Tabelle noch");

			// Now that the object is deleted, we can copy it from the fixed table.
			object_copy (fixed_table, editable_table, id);
			exists_editable=object_exists_in (editable_table, id);

			// If the object does not exist, this is an error.
			if (!exists_editable) throw ex_operation_failed ("Objekt existiert nach Kopieren in die editierbare Tabelle nicht");

			// Now that we know the object exists in the editable table, we can
			// delete it from the fixed table.
			object_delete_from (fixed_table, id);
			exists_fixed=object_exists_in (fixed_table, id);

			// If the object still exists, this is an error.
			if (exists_fixed) throw ex_operation_failed ("Objekt existiert nach L�schen aus fester Tabelle noch");
		}
		else
		{
			// The object does not exist in the fixed table. Nothing to do
			// because if it exists in the editable table, it is already
			// editable, and if not, this is an error.
		}
	}
	else
	{
		// Make not editable
		if (exists_fixed)
		{
			// Nothing to do because the object from the fixed table takes
			// precedence over the one in the editable table.
		}
		else
		{
			// The object does not exist in the fixed table.
			// Copy it there.
			// We can assume that the object exists in the editable table
			// because it is checked above that it exists in at least one of
			// the tables.
			object_copy (editable_table, fixed_table, id);
			exists_fixed=object_exists_in (fixed_table, id);

			// If the object does on exist in the fixed table now, this is an error.
			if (!exists_fixed) throw ex_operation_failed ("Objekt existiert nach Kopieren in die permanente Tabelle nicht");
		}
	}
}



// Writing
db_id sk_db::write_object (db_object_type type, void *object)
{
	// TODO this function is MURX.
	//
	// TODO Error reporting (exceptions), dann hier auch editierbar/nicht
	// editierbar machen. Dann aus sk_web::handler_do_edit_person make_editable
	// entfernen
	//
	// TODO: Fehlerbeschreibung!
	if (!connected ()) return invalid_id;	// Need to be connected to write
	if (type==ot_none) return invalid_id;	// Need an object type

	db_id object_id;
	bool editable_flag;
	// TODO: Das ist ziemlich unelegant...
	switch (type)
	{
		case ot_flight:
			object_id=((Flight *)object)->id;
			editable_flag=((Flight *)object)->editierbar;
			break;
		case ot_plane:
			object_id=((Plane *)object)->id;
			editable_flag=((Plane *)object)->editierbar;
			break;
		case ot_person:
			object_id=((Person *)object)->id;
			editable_flag=((Person *)object)->editierbar;
			break;
		default:
			debug_stream << "[db] Unhandled object type in sk_db::write_object ()" << std::endl;
			return invalid_id;
			break;
	}

	QString query;
	// If the object does not yet have an ID or this ID does not yet exist,
	// we have to create it, else we have to modify it.
	bool create=(id_invalid (object_id) || !object_exists (type, object_id));

	if (create)
	{
		// Object does not exist yet ==> create it.
		// By using editable_flag, the correct table (editable or fixed) is
		// selected by the make_query function.
		query=make_query (qt_create, type, "", query_value_list (type, object), false ,false, "", "", editable_flag);

		MYSQL_RES *result;
		if (execute_query (&result, query, true)==db_ok)
		{
			db_id r=mysql_insert_id (mysql);
			mysql_free_result (result);
			return r;
		}
		else
		{
			debug_stream << "There was an error while executing the query in sk_db::write_object (): " << get_last_error () << std::endl;
			return invalid_id;
		}
	}
	else
	{
		// Object exists ==> update it.
		int ret=db_ok;

		// Update the editable table.
		query=make_query (qt_update, type, make_condition (condition_t (cond_id, object_id)), query_value_list (type, object), false, false, "", "", true);
		ret=execute_query (query);
		if (ret!=db_ok) return invalid_id;

		// If this is an admin_db, we also need to update the fixed table.
		query=make_query (qt_update, type, make_condition (condition_t (cond_id, object_id)), query_value_list (type, object), false, false, "", "", false);
		ret=execute_query (query);
		if (ret!=db_ok) return invalid_id;

		return object_id;
	}
}

db_id sk_db::write_flight (Flight *p)
{
	return write_object (ot_flight, p);
}

db_id sk_db::write_person (Person *p)
{
	return write_object (ot_person, p);
}

db_id sk_db::write_plane (Plane *p)
{
	return write_object (ot_plane, p);
}


// Deletion
int sk_db::delete_object (db_object_type otype, db_id id)
{
	// TODO this function is a mess.

	if (!connected ()) return db_err_not_connected;

	// Check if this object exists at all.
	if (!object_exists (otype, id)) return db_err_not_found;

	QString condition=make_condition (condition_t (cond_id, id));
	QString columns=query_column_list (otype, true);

	// Query for deleting from the editable table
	QString query_editable=make_query (qt_delete, otype, condition, columns, false, false, "", "", true);
	// Query for deleting from the fixed table
	QString query_fixed=make_query (qt_delete, otype, condition, columns, false, false, "", "", false);

	int res=db_ok;
	int affected_rows_editable=-1, affected_rows_fixed=-1;

	// Try the delete command on the editable table
	if (res==db_ok)
	{
		MYSQL_RES *result;
		res=execute_query (&result, query_editable, true);
		if (res==db_ok)
		{
			affected_rows_editable=mysql_affected_rows (mysql);
			mysql_free_result (result);
		}
	}

	// Try the delete command on the fixed table if this is an admin_db
	if (res==db_ok && is_admin_db)
	{
		MYSQL_RES *result;
		res=execute_query (&result, query_fixed, true);
		if (res==db_ok)
		{
			affected_rows_fixed=mysql_affected_rows (mysql);
			mysql_free_result (result);
		}
	}

	// TODO might also use get_object before to check if it is editable.
	if (res==db_ok)
	{
		// As we checked above, we know that the object existed.

		// If this is not an admin_db and nothing happened, the object was not editable.
		if (!is_admin_db && affected_rows_editable==0) return db_err_not_editable;

		// TODO if admin_db and both affected_rows_editable and
		// affected_rows_fixed are 0, this is an error.

		return db_ok;
	}
	else
	{
		// An error occured, return it.
		return res;
	}
}

int sk_db::delete_flight (db_id id)
{
	return delete_object (ot_flight, id);
}

int sk_db::delete_person (db_id id)
{
	return delete_object (ot_person, id);
}

int sk_db::delete_plane (db_id id)
{
	return delete_object (ot_plane, id);
}


// Existance
int sk_db::num_results_query (QString query)
	// Executes a query and returns the number of results
{
	if (!connected ()) return db_err_not_connected;

	MYSQL_RES *result;
	if (execute_query (&result, query, true)==db_ok)
	{
		int num_rows=mysql_num_rows (result);
		mysql_free_result (result);
		return num_rows;
	}
	else
	{
		return db_err_query_failed;
	}
}

int sk_db::rows_exist_query (QString query)
{
	int res=num_results_query (query);

	if (res<0)
		return res;
	else if (res==0)
		return 0;
	else
		return 1;
}

int sk_db::object_exists (db_object_type type, db_id id)
{
	// Try to get the (id of the) object. If that returns rows, the object
	// exists.

	if (!connected ()) return db_err_not_connected;
	if (id_invalid (id)) return db_err_parameter_error;

	QString condition=make_condition (condition_t (cond_id, id));
	QString columns=query_column_list (type, true);
	QString query=make_query (qt_list, type, condition, columns, true, false);

	return rows_exist_query (query);
}

int sk_db::flight_exists (db_id id)
{
	return object_exists (ot_flight, id);
}

int sk_db::plane_exists (db_id id)
{
	return object_exists (ot_plane, id);
}

int sk_db::person_exists (db_id id)
{
	return object_exists (ot_person, id);
}


// Listing
int sk_db::result_to_list (db_object_type type, QPtrList<void> &result_list, MYSQL_RES *result)
{
	if (type==ot_none) return db_err_parameter_error;
	if (!result) return db_err_parameter_error;

	unsigned int num_fields=mysql_num_fields (result);
	MYSQL_FIELD *fields=mysql_fetch_fields (result);

	// For every row returned...
	MYSQL_ROW row;
	while ((row=mysql_fetch_row (result)))
	{
		void *object=new_object (type);

		// Decode the object and add it to the list if successful
		if (row_to_object (type, object, row, num_fields, fields)>=0)
			result_list.append (object);
		else
			free_object (type, object);
	}

	return db_ok;
}

int sk_db::result_to_num_list (QValueList<long long int> &nums, MYSQL_RES *result, const char *field_name)
{
	// TODO code duplication with result_to_list
	if (!result) return db_err_parameter_error;

	unsigned int num_fields=mysql_num_fields (result);
	MYSQL_FIELD *fields=mysql_fetch_fields (result);

	// For every row returned...
	MYSQL_ROW row;
	while ((row=mysql_fetch_row (result)))
	{
		// Decode the number and add it to the list if successful
		long long int num=row_to_number (row, num_fields, fields, field_name);
		nums.append (num);
	}

	return db_ok;
}

int sk_db::result_to_string_list (QStringList &strings, MYSQL_RES *result, const char *field_name)
{
	// TODO code duplication with result_to_list
	if (!result) return db_err_parameter_error;

	unsigned int num_fields=mysql_num_fields (result);
	MYSQL_FIELD *fields=mysql_fetch_fields (result);

	// For every row returned...
	MYSQL_ROW row;
	while ((row=mysql_fetch_row (result)))
		strings.append (std2q (named_field_value (row, num_fields, fields, field_name)));

	return db_ok;
}

int sk_db::result_to_string_list (QStringList &strings, MYSQL_RES *result, const unsigned int field_num)
{
	// TODO code duplication with result_to_list
	// TODO code duplication with other result_to_list functions
	if (!result) return db_err_parameter_error;

	unsigned int num_fields=mysql_num_fields (result);
	MYSQL_FIELD *fields=mysql_fetch_fields (result);

	// For every row returned...
	MYSQL_ROW row;
	while ((row=mysql_fetch_row (result)))
		strings.append (std2q (field_value (row, num_fields, fields, field_num)));

	return db_ok;
}

int sk_db::result_to_id_list (QValueList<db_id> &ids, MYSQL_RES *result)
{
	QValueList<long long int> temp_list;
	// TODO "id" hardcoded
	int r=result_to_num_list (temp_list, result, "id");
	if (r<0) return r;

	// Convert from long long int to db_id
	for (QValueListIterator<long long int> it=temp_list.begin (); it!=temp_list.end (); ++it)
		ids.append ((db_id)*it);

	return db_ok;
}

int sk_db::list_id_data (db_object_type type, QValueList<db_id> &ids, QStringList &data_columns, condition_t c)
	// This function returns IDs from multiple columns where the row matches
	// a given condition c.
	// Example: all values from "pilot" and "begleiter" from flights that
	// happened on a certain date.
{
	// TODO code duplication with list_objects
	if (!connected ()) return db_err_not_connected;

	QString condition=make_condition (c);
	// TODO move this functionality to make_query?
	// +: all information about queries would be contained there.
	// -: Need to pass the QString list in addition.
	// /: Make a multiple_query function. Would faciliate query_t class creation.
	QStringList query_parts;
	for (QStringList::Iterator data_column=data_columns.begin (); data_column!=data_columns.end (); ++data_column)
		query_parts.append (make_query (qt_list, type, condition, *data_column+" as id", false, true));
	QString query=query_parts.join (" UNION ");

	// TODO code duplications with list_ids (exact!)
	MYSQL_RES *result;
	if (execute_query (&result, query, true)==db_ok)
	{
		// Make a list from the IDs
		result_to_id_list (ids, result);
		mysql_free_result (result);

		return db_ok;
	}
	else
	{
		return db_err_query_failed;
	}
}

int sk_db::list_ids (db_object_type type, QValueList<db_id> &ids, condition_t c)
	// This function returns the IDs of table rows matching a given condition
	// c.
	// Example: the IDs of all flights that happened on a certain date.
{
	QStringList data_columns;
	// Murx: query_column_list, das nur "id" zur�ckgibt, besser column_name_id hier verwenden.
	data_columns.append (query_column_list (type, true));
	return list_id_data (type, ids, data_columns, c);
}

int sk_db::list_strings (db_object_type type, QString field_name, QStringList &strings, condition_t c, QString custom_query)
{
	// TODO use list_strings_query
	// TODO code duplication with list_objects
	if (!connected ()) return db_err_not_connected;

	QString query;
	// MURX: some of the QString listing queries need a custom query
	if (custom_query.isEmpty ())
	{
		QString condition=make_condition (c);
		// Do not include the editable column because if there are multiple
		// entries matching, some editable, some not, they would not be
		// identical and thus would show up twice in the output.
		query=make_query (qt_list, type, condition, field_name, false, false, field_name);
	}
	else
	{
		query=custom_query;
	}

	MYSQL_RES *result;
	if (execute_query (&result, query, true)==db_ok)
	{
		// Make a list from the strings
		result_to_string_list (strings, result, field_name.latin1());
		mysql_free_result (result);

		return db_ok;
	}
	else
	{
		return db_err_query_failed;
	}
}

int sk_db::list_objects (db_object_type type, QPtrList<void> &objects, condition_t c)
{
	if (!connected ()) return db_err_not_connected;

	QString condition=make_condition (c);
	QString columns=query_column_list (type, false);
	QString query=make_query (qt_list, type, condition, columns, true, false, default_sort_column (type));

	MYSQL_RES *result;
	if (execute_query (&result, query, true)==db_ok)
	{
		// Make a list from the objects
		result_to_list (type, objects, result);
		mysql_free_result (result);

		return db_ok;
	}
	else
	{
		return db_err_query_failed;
	}
}

int sk_db::list_flights (QPtrList<Flight> &flights, condition_t c)
{
	QPtrList<void> objects;
	int r=list_objects (ot_flight, objects, c);
	// MURX Zeiger umkopieren, weil die Listentypen nicht konvertibel sind.
	for (QPtrListIterator<void> ob (objects); *ob; ++ob) flights.append ((Flight *)*ob);
	return r;
}

int sk_db::list_planes (QPtrList<Plane> &planes, condition_t c)
{
	QPtrList<void> objects;
	int r=list_objects (ot_plane, objects, c);
	// MURX Zeiger umkopieren, weil die Listentypen nicht konvertibel sind.
	for (QPtrListIterator<void> ob (objects); *ob; ++ob) planes.append ((Plane *)*ob);
	return r;
}

int sk_db::list_persons (QPtrList<Person> &persons, condition_t c)
{
	QPtrList<void> objects;
	int r=list_objects (ot_person, objects, c);
	// MURX Zeiger umkopieren, weil die Listentypen nicht konvertibel sind.
	for (QPtrListIterator<void> ob (objects); *ob; ++ob) persons.append ((Person *)*ob);
	return r;
}

int sk_db::list_strings_query (const QString query, const QString field_name, QStringList &strings)
{
	// TODO code duplication with other list_strings_query
	if (!connected ()) return db_err_not_connected;

	MYSQL_RES *res;
	if (execute_query (&res, query, true)==db_ok)
	{
		// Make a list from the strings
		result_to_string_list (strings, res, field_name.latin1());
		mysql_free_result (res);

		return db_ok;
	}
	else
	{
		return db_err_query_failed;
	}
}

int sk_db::list_strings_query (const QString query, const unsigned int field_num, QStringList &strings)
{
	// TODO code duplication with other list_strings_query
	if (!connected ()) return db_err_not_connected;

	MYSQL_RES *res;
	if (execute_query (&res, query, true)==db_ok)
	{
		// Make a list from the strings
		result_to_string_list (strings, res, field_num);
		mysql_free_result (res);

		return db_ok;
	}
	else
	{
		return db_err_query_failed;
	}
}



// Listing Frontends
int sk_db::list_flights_date (QPtrList<Flight> &flights, QDate *date)
{
	int r=list_flights (flights, condition_t (cond_flight_happened_on_date, date));
	return r;
}

int sk_db::list_flights_date_range (flight_list &flights, QDate *start_date, QDate *end_date)
{
	int r=list_flights (flights, condition_t (cond_flight_happened_between, start_date, end_date));
	return r;
}

int sk_db::list_flights_prepared (QPtrList<Flight> &flights)
{
	int r=list_flights (flights, condition_t (cond_flight_prepared));
	return r;
}

db_id sk_db::object_flying (db_object_type otype, db_id id, sk_time_t *given_time)
{
	if (id_invalid (id)) return invalid_id;

	condition_type_t cond;
	switch (otype)
	{
		case ot_person: cond=cond_flight_person_flying_at; break;
		case ot_plane: cond=cond_flight_plane_flying_at; break;
		default:
			debug_stream << "[db] Error: inappropriate object type in object_flying ()" << std::endl;
			cond=cond_none; break;
	}

	condition_t c (cond, id);
	c.given_time1=given_time;

	QValueList<db_id> ids;
	int res=list_ids (ot_flight, ids, c);

	if (res!=db_ok) return false;

	if (ids.isEmpty ())
		return invalid_id;
	else
		// TODO newest...
		return ids.first ();
}

db_id sk_db::person_flying (db_id id, sk_time_t *given_time)
{
	return object_flying (ot_person, id, given_time);
}

db_id sk_db::plane_flying (db_id id, sk_time_t *given_time)
{
	return object_flying (ot_plane, id, given_time);
}

int sk_db::list_persons_by_name (QPtrList<Person> &names, QString vorname, QString nachname)
{
	return list_persons (names, condition_t (cond_person_name, &vorname, &nachname));
}

int sk_db::list_persons_by_first_name (QPtrList<Person> &names, QString vorname)
{
	return list_persons (names, condition_t (cond_person_first_name, &vorname));
}

int sk_db::list_persons_by_last_name (QPtrList<Person> &names, QString nachname)
{
	return list_persons (names, condition_t (cond_person_last_name, &nachname));
}

int sk_db::list_persons_by_club_club_id (QPtrList<Person> &persons, QString club, QString club_id)
{
	return list_persons (persons, condition_t (cond_person_club_club_id, &club, &club_id));
}

int sk_db::list_planes_all (QPtrList<Plane> &planes)
{
	return list_planes (planes, condition_t (cond_any));
}

int sk_db::list_persons_all (QPtrList<Person> &planes)
{
	return list_persons (planes, condition_t (cond_any));
}

int sk_db::list_planes_date (QPtrList<Plane> &planes, QDate *date)
{
	// TODO code duplication list_persons_date

	// TODO consider splitting this function into one returning an ID list and
	// one returning actual objects. Probably put the copying code not in the
	// latter but in a separate function (get_persons (person_list, id_list)).
	condition_t cond (cond_flight_happened_on_date, date);
	QValueList<db_id> id_list;

	QStringList column_list;
	column_list.append (column_name_flug_flugzeug);

	int r=list_id_data (ot_flight, id_list, column_list, cond);
	// TODO error handling
	for (QValueListIterator<db_id> id=id_list.begin (); id!=id_list.end (); ++id)
	{
		// ID==0 is OK as the data column may continue this to signify "none".
		// However, we don't want to include it in the output.
		if (!id_invalid (*id))
		{
			Plane *plane=new Plane;

			int r=get_plane (plane, *id);
			if (r==db_ok)
				planes.append (plane);
			else
				delete plane;
		}
	}
	return r;
}

int sk_db::list_persons_date (QPtrList<Person> &persons, QDate *date)
{
	/*
	 * //TODO these are generic notes, put them to the documentation.
	 * This function first lists all IDs in question and then gets every object
	 * separately. Interestingly, this is more than 50 times faster than the
	 * commented out code (measured with 5 flights involving 3 people).
	 * The old code is more elegant as there is only one query containing all
	 * the information and returning the correct objects.
	 * Furthermore, you could also generate a club list that way without
	 * manually checking for duplicates.
	 */
	// The following is the old (slow) code:
	// This also only uses pilot. We need person (pilot OR copilot).
	//condition_t cond (cond_flight_happened_on_date, date);
	//cond.set_meta_query (ot_flight, &column_name_flug_pilot);
	//return list_persons (list, cond);

	// TODO consider splitting this function into one returning an ID list and
	// one returning actual objects. Probably put the copying code not in the
	// latter but in a separate function (get_persons (person_list, id_list)).
	condition_t cond (cond_flight_happened_on_date, date);
	QValueList<db_id> id_list;

	QStringList column_list;
	column_list.append (column_name_flug_pilot);
	column_list.append (column_name_flug_begleiter);

	int r=list_id_data (ot_flight, id_list, column_list, cond);
	// TODO error handling
	for (QValueListIterator<db_id> id=id_list.begin (); id!=id_list.end (); ++id)
	{
		// ID==0 is OK as the data column may continue this to signify "none".
		// However, we don't want to include it in the output.
		if (!id_invalid (*id))
		{
			Person *person=new Person;

			int r=get_person (person, *id);
			if (r==db_ok)
				persons.append (person);
			else
				delete person;
		}
	}
	return r;
}

int sk_db::list_planes_registration (QPtrList<Plane> &planes, QString registration)
{
	return list_planes (planes, condition_t (cond_plane_registration, &registration));
}

int sk_db::get_plane_registration (Plane *plane, QString registration)
{
	QPtrList<Plane> planes; planes.setAutoDelete (true);
	int res=list_planes_registration (planes, registration);

	// TODO handle multiple
	// TODO better list IDs here and get by the chosen ID (?)
	if (res==db_ok && !planes.isEmpty ())
		*plane=*(planes.first ());

	if (planes.isEmpty ()) res=db_err_not_found;

	return res;
}


// String lists
//	return list_strings (ot_, "(column name)", (QString list), condition_t (cond_));
int sk_db::list_airfields (QStringList &airfields)
{
	QString custom_query;
	custom_query+="select startort as ort from "+fixed_table_name (ot_flight);
	custom_query+=" UNION select zielort as ort from "+fixed_table_name (ot_flight);
	custom_query+=" UNION select startort as ort from "+editable_table_name (ot_flight);
	custom_query+=" UNION select zielort as ort from "+editable_table_name (ot_flight)+" group by ort";
	return list_strings (ot_none, "ort", airfields, condition_t (), custom_query);
}

int sk_db::list_first_names (QStringList &names, QString last_name)
{
	if (last_name.isEmpty ())
		return list_strings (ot_person, "vorname", names, condition_t (cond_any));
	else
		return list_strings (ot_person, "vorname", names, condition_t (cond_person_last_name, &last_name));
}

int sk_db::list_last_names (QStringList &names, QString first_name)
{
	if (first_name.isEmpty ())
		return list_strings (ot_person, "nachname", names, condition_t (cond_any));
	else
		return list_strings (ot_person, "nachname", names, condition_t (cond_person_first_name, &first_name));
}

int sk_db::list_accounting_note (QStringList &notes)
{
	return list_strings (ot_flight, "abrechnungshinweis", notes, condition_t (cond_any));
}

int sk_db::list_registrations (QStringList &registrations)
{
	return list_strings (ot_plane, "kennzeichen", registrations, condition_t (cond_any));
}

int sk_db::list_types (QStringList &types)
{
	return list_strings (ot_plane, "typ", types, condition_t (cond_any));
}

int sk_db::list_clubs (QStringList &clubs)
{
	QString custom_query;
	custom_query+="select verein from "+fixed_table_name (ot_flight);
	custom_query+=" UNION select verein from "+editable_table_name (ot_flight);
	custom_query+=" UNION select verein from "+fixed_table_name (ot_person);
	custom_query+=" UNION select verein from "+editable_table_name (ot_person)+" group by verein";
	return list_strings (ot_none, "verein", clubs, condition_t (), custom_query);
	return db_err_not_implemented;
}





// Counting
long long int sk_db::count_objects (db_object_type type, condition_t c)
{
	// TODO code duplication with list_objects
	if (!connected ()) return db_err_not_connected;

	QString condition=make_condition (c);
	QString query=make_query (qt_count_murx, type, condition, "", false, false);

	MYSQL_RES *result;
	if (execute_query (&result, query, true)==db_ok)
	{
		// Make a list from the objects
		QValueList<long long int> nums;
		result_to_num_list (nums, result, column_name_count.latin1 ());
		mysql_free_result (result);

		long long int sum=0;
		for (QValueListIterator<long long int> it=nums.begin (); it!=nums.end (); ++it)
			sum+=*it;
		return sum;
	}
	else
	{
		return db_err_query_failed;
	}
}

long long int sk_db::count_flights (condition_t c)
{
	return count_objects (ot_flight, c);
}

long long int sk_db::count_planes (condition_t c)
{
	return count_objects (ot_plane, c);
}

long long int sk_db::count_persons (condition_t c)
{
	return count_objects (ot_person, c);
}


// Counting frontends
long long int sk_db::count_flights_current (QDate &date)
{
	return count_flights (condition_t (cond_flight_will_land, &date));
}

long long int sk_db::count_flights_today (QDate &date)
{
	return count_flights (condition_t (cond_flight_happened_on_date, &date));
}

// TODO all these functions lack proper error handling
bool sk_db::object_has_flight (db_object_type otype, db_id id)
{
	condition_type_t crit;

	switch (otype)
	{
		case ot_person: crit=cond_flight_person; break;
		case ot_plane: crit=cond_flight_plane_or_towplane; break;
		default:
			debug_stream << "[db] Error: inappropriate object type in sk_db" << std::endl;
			crit=cond_none;
			break;
	}

	return (count_flights (condition_t (crit, id))>0);
}

bool sk_db::person_has_flight (db_id id)
{
	return object_has_flight (ot_person, id);
}

bool sk_db::person_has_user (db_id id)
{
	int ret=rows_exist_query ("select username from user where person="+QString::number (id));

	if (ret<0)
		return true;	// Fail to the safe side TODO proper error handling
	else if (ret>0)
		return true;
	else
		return false;
}

bool sk_db::plane_has_flight (db_id id)
{
	return object_has_flight (ot_plane, id);
}

bool sk_db::person_used (db_id id)
{
	return (person_has_flight (id) || person_has_user (id));
}

bool sk_db::plane_used (db_id id)
{
	return (plane_has_flight (id));
}



// Get by ID
int sk_db::get_object (db_object_type type, void *object, db_id id)
	// type and the type of object must match!
{
	if (!connected ()) return db_err_not_connected;
	if (id_invalid (id)) return db_err_parameter_error;

	if (object)
	{
		// Get the object
		QPtrList<void> objects; objects.setAutoDelete (true);
		int ret=list_objects (type, objects, condition_t (cond_id, id));

		if (ret==db_ok)
		{
			if (objects.isEmpty ())
			{
				// Object not found
				return db_err_not_found;
			}
			else
			{
				if (objects.count ()>1)
					debug_stream << "[db] Database Error: multiple " << object_name (type, true) << " with id " << id << " found." << std::endl;

				// Object found, return it
				// TODO handle the error that there are multiple objects with his ID.
				// Then, remove the sorting/grouping from the query.
				copy_object (type, object, objects.first ());
				return db_ok;
			}
		}
		else
		{
			return ret;
		}
	}
	else
	{
		// Check for existance only
		QValueList<db_id> id_list;
		int ret=list_ids (type, id_list, condition_t (cond_id, id));
		if (ret==db_ok)
		{
			if (id_list.isEmpty ())
				return db_err_not_found;
			else
				return db_ok;
		}
		else
		{
			return ret;
		}
	}
}

int sk_db::get_flight (Flight *flight, db_id id)
{
	return get_object (ot_flight, flight, id);
}

int sk_db::get_plane (Plane *plane, db_id id)
{
	return get_object (ot_plane, plane, id);
}

int sk_db::get_person (Person *person, db_id id)
{
	return get_object (ot_person, person, id);
}




// Startart - to be sorted

int sk_db::count_startart ()
{
	return startarten.count ();
}

bool sk_db::add_startart_to_list (LaunchType *sa)
{
	// TODO some types must be unique, for example sat_self

	// TODO Fehlermeldungen: Bezeichnung der Startart dazu.
	if (id_invalid (sa->get_id ()))
	{
		log_error ("Invalid startart id in add_startart_to_list");
		return false;
	}
	else if (get_startart (NULL, sa->get_id ())==db_ok)
	{
		log_error ("Startart bereits vorhanden in sk_db::add_startart_to_list ()");
		return false;
	}
	else
	{
		sa->ok=true;
		startarten.append (sa);
		return true;
	}
}

int sk_db::list_startarten_all (QPtrList<LaunchType> &saen)
{
	for (QPtrListIterator<LaunchType> sa (startarten); *sa; ++sa)
		saen.append (new LaunchType (**sa));

	return true;
}

int sk_db::get_startart (LaunchType *startart, db_id id)
{
	for (QPtrListIterator<LaunchType> sa (startarten); *sa; ++sa)
	{
		if ((*sa)->get_id ()==id)
		{
			if (startart)
			{
				(*startart)=**sa;
				startart->ok=true;
			}
			return db_ok;
		}
	}

	return db_err_not_found;
}

int sk_db::get_startart_by_type (LaunchType *startart, startart_type sat)
{
	bool found=false;

	for (QPtrListIterator<LaunchType> sa (startarten); *sa; ++sa)
	{
		if ((*sa)->get_type ()==sat)
		{
			if (startart)
			{
				(*startart)=**sa;
				startart->ok=true;
			}
			found=true;
			break;
		}
	}

	return found?db_ok:db_err_not_found;
}

db_id sk_db::get_startart_id_by_type (startart_type sat)
{
	for (QPtrListIterator<LaunchType> sa (startarten); *sa; ++sa)
		if ((*sa)->get_type ()==sat)
			return (*sa)->get_id ();

	return invalid_id;
}

int sk_db::get_towplane (Plane *towplane, const LaunchType &startart, const db_id towplane_id)
{
	if (startart.ok && startart.is_airtow () && startart.towplane_known ())
	{
		// Get the tow plane from the startart
		return get_plane_registration (towplane, startart.get_towplane ());
	}
	else
	{
		// The tow plane is not known from the startart, so we have to get
		// it from the explicitly given towplane_id.
		return get_plane (towplane, towplane_id);
	}
}



// Merging persons
QString merge_person_query (QString table, QString column, db_id correct, db_id wrong)
{
	// update table set $column=$correct where $column=$wrong
	return "update "+table+" set "+column+"="+QString::number (correct)+" where "+column+"="+QString::number (wrong);
}

void sk_db::merge_person (db_id correct_id, db_id wrong_id)
	throw (ex_not_connected, ex_parameter_error, ex_query_failed, ex_operation_failed)
{
	// Check some preconditions.
	if (!connected ()) throw ex_not_connected ();
	if (id_invalid (correct_id)) throw ex_parameter_error ("Korrekte ID ist ung�ltig");
	if (id_invalid (wrong_id)) throw ex_parameter_error ("Falsche ID ist ung�ltig");

	// Change the person everywhere it occurs. This can be the flight tables,
	// columns pilot and begleiter.
	if (!silent) std::cout << "Changing the person" << std::endl;
	if (execute_query (merge_person_query ("flug", column_name_flug_pilot, correct_id, wrong_id))!=db_ok) throw ex_query_failed (last_query);
	if (execute_query (merge_person_query ("flug", column_name_flug_begleiter, correct_id, wrong_id))!=db_ok) throw ex_query_failed (last_query);
	if (execute_query (merge_person_query ("flug", column_name_flug_towpilot, correct_id, wrong_id))!=db_ok) throw ex_query_failed (last_query);
	if (execute_query (merge_person_query ("flug_temp", column_name_flug_pilot, correct_id, wrong_id))!=db_ok) throw ex_query_failed (last_query);
	if (execute_query (merge_person_query ("flug_temp", column_name_flug_begleiter, correct_id, wrong_id))!=db_ok) throw ex_query_failed (last_query);
	if (execute_query (merge_person_query ("flug_temp", column_name_flug_towpilot, correct_id, wrong_id))!=db_ok) throw ex_query_failed (last_query);
	if (execute_query ("update user set person="+QString::number (correct_id)+" where person="+QString::number (wrong_id))!=db_ok) throw ex_query_failed (last_query);

	// Now check if there are still any persons with that ID
	// Select flights which have that person.
	if (!silent) std::cout << "Checking success" << std::endl;
	QValueList<db_id> flight_ids;
	condition_t cond (cond_flight_person, wrong_id);
	if (list_ids (ot_flight, flight_ids, cond)!=db_ok) throw ex_operation_failed (get_last_error ());
	// This list better be empty.
	if (!flight_ids.isEmpty ())
	{
		debug_stream << "Error: after changing the IDs, there are still flights with the wrong ID " << wrong_id <<":" << std::endl;

		for (QValueListIterator<db_id> it=flight_ids.begin (); it!=flight_ids.end (); ++it)
		{
			if (it!=flight_ids.begin ()) debug_stream << ", ";
			debug_stream << *it;
		}
		debug_stream << std::endl;

		throw ex_operation_failed ("Wrong person still exists in flights");
	}
	// Select users which have that person.
	int ret=rows_exist_query ("select username from user where person="+QString::number (wrong_id));
	if (ret<0) throw ex_query_failed (last_query);
	if (ret>0)
	{
		debug_stream << "Error: after changing the IDs, there are still users with the wrong ID " << wrong_id <<"." << std::endl;
		throw ex_operation_failed ("Wrong person still exists in users");
	}

	// Now that there are no entries with this person any more, we can safely
	// delete it.
	if (!silent) std::cout << "Deleting wrong person" << std::endl;
	if (person_exists (wrong_id))
	{
		if (delete_person (wrong_id)!=db_ok) throw ex_operation_failed (get_last_error ());
	}
}



// Importing
void sk_db::remove_editable_persons (QPtrList<Person> persons)
{
	QPtrListIterator<Person> it (persons);
	while (*it)
	{
		if ((*it)->editierbar)
			persons.remove (*it);
		else
			++it;
	}
}

void sk_db::import_check (const Person &person)
	throw (import_message)
{
	// First name is empty
	if (person.vorname.isEmpty ()) throw import_message::first_name_missing ();

	// Last name is empty
	if (person.nachname.isEmpty ()) throw import_message::last_name_missing ();
}

void sk_db::import_check (const QPtrList<Person> &persons, QList<import_message> &messages)
	// This function make be slow because it takes quadratic time (in the
	// number of persons) with lots of QString comparisons.
{
	// This checks every single person from the list and additionally performs
	// cross checking (like club ID uniquity).

	for (QPtrListIterator<Person> p1 (persons); *p1; ++p1)
	{
		// Check person
		try
		{
			import_check (**p1);
		}
		catch (import_message &n)
		{
			n.set_p1 (*p1);
			messages.push_back (n);
		}

		// Cross checks
		for (QPtrListIterator<Person> p2 (persons); *p2; ++p2)
			// Don't start at p1 here because there may be error relations
			// which are not symmetric: for example, two persons with the same
			// name only one of which has a club ID. This is an error for the
			// person without, but not for the one with club ID.
		{
			if (p1!=p2)
			{
				// Same club and club ID (except "")
				if (
					!(*p1)->club_id.isEmpty () &&
					(*p1)->club == (*p2)->club &&
					(*p1)->club_id == (*p2)->club_id
					)
					messages.push_back (import_message::duplicate_club_id (*p1, *p2));

				// Non-unique names without club_id
				if (
					(*p1)->club_id.isEmpty () &&
					(*p1)->vorname == (*p2)->vorname &&
					(*p1)->nachname == (*p2)->nachname
					)
					messages.push_back (import_message::duplicate_name_without_club_id (*p1, *p2));
			}
		}
	}
}

db_id sk_db::import_identify (const Person &p, QList<import_message> *non_fatal_messages)
	throw (ex_not_connected, ex_legacy_error, ex_operation_failed, import_message)
	/*
	 * Identify a person for importing.
	 * Paramters:
	 *   - p: the data used for identification
	 * Parameters set:
	 *   - non_fatal_messages: If not NULL, non-fatal problems are added here.
	 *     Fatal problems are thrown.
	 * Return value:
	 *   The ID of the person identified or invalid_id if the person is to be
	 *   created.
	 * Exceptions:
	 *   - import_message: A fatal problem occured. None-fatal problems are
	 *     added to the list.
	 */
{
	// OK, this function is quite long and complex. I thought long and hard
	// about it, and I rewrote everything 3 times trying to get some sutrcture
	// into it. However, it is still a bit of a mess. It may still have bugs as
	// there's an awful lot of cases to consider and in some cases it is not
	// quite clear what should happen at all.
	//
	// How persons can be identified:
	//   - club_id_old (using club, club_id_old)
	//   - club_id (using club, club_id)
	//   - name (using club, last name, first name)
	//   The first identification method present is used.
	//   For editable persons, the club ID is never used. To be more exact:
	//   when selecting by club ID (or old club ID), editable persons aren't
	//   considered. When selecting by name, nothing needs to be done because
	//   club IDs aren't used. TODO: is this good?
	// Procedure:
	//   - If club_id_old is given, a person with this club ID must exist. If
	//     it does, it is used. If not, an error is returned.
	//   - Otherwise, if club_id is given and a person with this club ID
	//     exists, it is used.
	//   - Otherwise, the person is selected according to its name. This is
	//     quite complicated because many different cases have to be checked,
	//     see below.
	// When a person has been identified, its ID is returned (or invalid_id for
	// "create new").

	bool club_id_old_given=!p.club_id_old.isEmpty ();
	bool club_id_given=!p.club_id.isEmpty ();

	// Do we need to select the person by name?
	bool select_by_name=false;

	if (club_id_old_given)
	{
		// An old club ID was explicitly given. This means that the person
		// identified by this club ID must exist. Everything else is an error.
		QPtrList<Person> persons; persons.setAutoDelete (true);
		int ret=list_persons_by_club_club_id (persons, p.club, p.club_id_old);
		if (ret!=0) throw ex_legacy_error (ret, *this);
		remove_editable_persons (persons);

		if (persons.isEmpty ())
			// No person with this club ID was found. This is an error.
			throw import_message::club_id_old_not_found ();
		else if (persons.count ()==1)
			// Exactly one person was found. Return it.
			return persons.getFirst ()->id;
		else
			// There were multiple objects found. To prevent accidental damage,
			// this is an error.
			throw import_message::club_id_not_unique ();
	}
	else if (club_id_given)
	{
		// A club ID was given. If it does not exist, the person has to be
		// created. If it does, the person with this club_id will be modified.
		QPtrList<Person> persons; persons.setAutoDelete (true);
		int ret=list_persons_by_club_club_id (persons, p.club, p.club_id);
		if (ret!=0) throw ex_legacy_error (ret, *this);
		remove_editable_persons (persons);

		if (persons.isEmpty ())
			// No person with this club ID was found. Either the person is new
			// or no club ID is used. Try to select the person by name.
			select_by_name=true;
		else if (persons.count ()==1)
			// Exactly one person was found. Return it.
			return persons.getFirst ()->id;
		else
			// There were multiple objects found. To prevent accidental dmage,
			// this is an error.
			throw import_message::club_id_not_unique ();
	}
	else
	{
		// No club ID or old club ID was given. This means that no club ID is
		// to be used, so the person has to be selected by name.
		select_by_name=true;
	}

	if (select_by_name)
	{
		// We need to select the person by name.
		QPtrList<Person> persons; persons.setAutoDelete (true);
		int ret=list_persons_by_name (persons, p.vorname, p.nachname);
		if (ret!=0) throw ex_legacy_error (ret, *this);

		if (persons.isEmpty ())
			// No person with this name exists. This means that the person has
			// to be newly created.
			return invalid_id;
		else
		{
			// Now this is a little bit complicated.
			// Let me summarize:
			//   - No club ID or old club ID was given. This means that either
			//     the person is to be used without club ID (this is allowed)
			//     or the person should have a club ID, but it is not already
			//     present.
			//   - There is a person with this name in the database.
			// Now, what could the persons we found be:
			//   1. A fixed person of another club
			//      We must not modify such a person. This means that we can
			//      remove all of these persons from the list right away.
			//   2. A fixed person of the own club with club ID.
			//      As this person has a club ID while the person imported does
			//      not, such a person is not used (to remove a club ID, the
			//      person must be selected with club_id_old, in which case we
			//      aren't here at all). It is removed as well.
			//      // TODO if the list is empty now, return invalid_id.
			//   3. A fixed person of the own club without club ID.
			//      If such a person is present, it is used. If there are
			//      multiple such persons, a not-unique error is returned.
			//   4. An editable person.
			//      In this case the club given cannot be trusted. If there is
			//      only one such person, it is used.
			//      4a: The club does not match
			//      4b: The club matches
			//      If there are multiple cat 4 persons but only one 4b, this
			//      one is used.
			//      If there are multiple 4b persons or multiple 4a and no 4b
			//      persons, a not-unique error is returned.

			// As stated above, remove all
			//   - fixed persons of a different club (type 1.)
			//   - fixed persons of the same club which have a club_id (type
			//     2.)
			// Also count the number of persons from categories 3, 4 and 4b.
			int num_fixed_own_no_club_id=0; db_id id_fixed_own_no_club_id=invalid_id; QString club_fixed_own_no_club_id;
			int num_editable=0;             db_id id_editable=invalid_id;             QString club_editable;
			int num_editable_own=0;         db_id id_editable_own=invalid_id;         QString club_editable_own;
			QPtrListIterator<Person> it (persons);
			while (*it)
			{
				// Remove the person if category 1. or 2.
				if (
						( !(*it)->editierbar       && (*it)->club!=p.club) ||	// 1.
						( !(*it)->club_id.isEmpty () && (*it)->club==p.club)		// 2.
				   )
				{
					persons.remove (*it);
					// The iterator it will now point to the item after the
					// removed one, which also has to be checked, so don't
					// increment it here.
					// If the item was the last one, it now points to the (now)
					// last item which will be checked again. This is not a
					// serious problem, but it's not good.
					// A solution would be to traverse the list backwards, but
					// QPtrList doesn't seem to support this.
				}
				else
				{
					// Not removed.

					// Category 3: fixed, own club, no club_id
					if (!(*it)->editierbar && (*it)->club==p.club && (*it)->club_id.isEmpty ())
					{
						num_fixed_own_no_club_id++;
						id_fixed_own_no_club_id=(*it)->id;
						club_fixed_own_no_club_id=(*it)->club;
					}

					// Category 4 (4a or 4b): editable
					if ((*it)->editierbar)
					{
						num_editable++;
						id_editable=(*it)->id;
						club_editable=(*it)->club;
					}

					// Category 4b: editable, own club
					if ((*it)->editierbar && (*it)->club==p.club)
					{
						num_editable_own++;
						id_editable_own=(*it)->id;
						club_editable_own=(*it)->club;
					}

					// As the persons was not removed, increment the iterator.
					++it;
				}

			}

			//DEBUG_OUTPUT (num_fixed_own_no_club_id);
			//DEBUG_OUTPUT (num_editable);
			//DEBUG_OUTPUT (num_editable_own);

			if (num_fixed_own_no_club_id==1)	// One cat. 3
			{
				// One fixed own person without club ID with this name. Use it.
				if (non_fatal_messages && club_fixed_own_no_club_id!=p.club) non_fatal_messages->push_back (import_message::club_mismatch (&p, club_fixed_own_no_club_id));
				return id_fixed_own_no_club_id;
			}
			else if (num_fixed_own_no_club_id>1)	// Multiple cat. 3
				// Multiple fixed own persons with that name. We don't known
				// which one to use ==> error.
				throw import_message::multiple_own_persons_name ();
			else if (num_editable==1)	// One cat. 4
			{
				// One editable person with that name. Use it.
				if (non_fatal_messages && club_editable!=p.club) non_fatal_messages->push_back (import_message::club_mismatch (&p, club_editable));
				return id_editable;
			}
			else if (num_editable==0)	// No cat. 4
				// No editable persons with that name. Create the person.
				return invalid_id;
			else if (num_editable_own==1)	// Multipe cat. 4, one cat. 4b
			{
				// Multiple editable persons with that name, but only one of
				// the own club. Use it.
				if (non_fatal_messages && club_editable_own!=p.club) non_fatal_messages->push_back (import_message::club_mismatch (&p, club_editable_own));
				return id_editable_own;
			}
			else if (num_editable_own>1)	// Multiple cat. 4, multiple 4b
				// Multiple editable persons with this name and the own club.
				// We don't know which one to use ==> error.
				throw import_message::multiple_editable_persons_name ();
			else	// Multiple cat. 4, no 4b
				// Multiple editable persons with that name, none of our club.
				// We don't know which one to use ==> error.
				throw import_message::multiple_editable_persons_name ();
		}
	}

	//TODO replace by internal_error
	throw ex_operation_failed ("Unbehandelter Fall in sk_db::import_identify ()");
}

void sk_db::import_identify (QPtrList<Person> &persons, QList<import_message> &messages)
	throw (ex_not_connected, ex_legacy_error, ex_operation_failed)
{
	for (QPtrListIterator<Person> it (persons); *it; ++it)
	{
		try
		{
			// Don't add the ID here as it is unclear what to do for errors.
			db_id id=import_identify (**it, &messages);
			(*it)->id=id;
		}
		catch (import_message &n)
		{
			(*it)->id=0;
			n.set_p1 (*it);
			messages.push_back (n);
		}
	}
}

db_id sk_db::import_person (const Person &person)
	throw (ex_not_connected, ex_legacy_error, ex_operation_failed)
{
	if (!connected ()) throw ex_not_connected ();

	// See also doc/importing
	// The persons given should have been checked before (this includes
	// checking whether they can be identified). If an error occurs, you are
	// thrown an exception.
	//
	// Procedure:
	//   1. Identify the person (the result may be "none", for "new person")
	//   2. Modify (or create) the person and set it to "not editable'.

	// Step 1: Identify the person
	db_id id=import_identify (person);

	// Step 2: Write the person to the database
	// Make a copy so we don't have to overwrite the parameter.
	Person p=person;
	p.id=id;
	// Write the person to the database
	int result_id=write_person (&p);
	if (id_invalid (result_id)) throw ex_operation_failed ("Person schreiben, Ergebnis: "+QString::number (result_id));

	// If the person was written successfully, make it not editable
	if (!id_invalid (result_id)) make_person_editable (result_id, false);

	return result_id;
}

void sk_db::import_persons (const QPtrList<Person> &persons)
	throw (ex_not_connected, ex_legacy_error, ex_operation_failed)
{
	if (!connected ()) throw ex_not_connected ();

	for (QPtrListIterator<Person> it (persons); *it; ++it)
		import_person (**it);
}




// User management
// Cannot use the "generic" functions because user is not a Entity
// (does not have an ID etc.). Also, the table is not duplicated
// (_temp);
// Code duplication with the "generic" functions.
int sk_db::sk_user_exists (const QString &username)
	/*
	 * Finds out if a user with a given name exists.
	 * Parameters:
	 *   - username: the name to check
	 * Return value:
	 *   - db_err_* on error.
	 *   - 0 if the user does not exist.
	 *   - 1 if the user exists.
	 */
{
	return rows_exist_query ("SELECT username FROM user WHERE username='"+escape (username)+"'");
}

int sk_db::sk_user_authenticate (const QString &username, const QString &password)
	/*
	 * Finds out if the user can login with a given username and password.
	 * Parameters:
	 *   - username: the user name
	 *   - password: the password, in clear text
	 * Return value:
	 *   - db_err_* on error
	 *   - 0 if authentication failed
	 *   - 1 if authentication succeeded
	 */
{
	set_query_display_alias ("SELECT username FROM user WHERE username='"+escape (username)+"' AND password=PASSWORD ('***')");
	return rows_exist_query ("SELECT username FROM user WHERE username='"+escape (username)+"' AND password=PASSWORD ('"+password+"')");
}

int sk_db::sk_user_change_password (const QString &username, const QString &password)
	/*
	 * Changes the password for a user.
	 * Parameters:
	 *   - user: the user to change
	 *   - password: the new password, in clear text
	 * Return value:
	 *   - db_err_* on error
	 *   - db_ok on success
	 */
{
	QString condition="username='"+escape (username)+"'";
	set_query_display_alias ("UPDATE user SET password=PASSWORD ('***') WHERE "+condition);
	return execute_query ("UPDATE user SET password=PASSWORD ('"+escape (password)+"') WHERE "+condition);
}

int sk_db::sk_user_add (const User &user, const QString &password)
	/*
	 * Adds a user.
	 * Parameters:
	 *   - user: the user to add
	 *   - password: the password to set, in cleartext.
	 * Return value:
	 *   - db_err_* on error
	 *   - db_ok on success.
	 */
{
	set_query_display_alias ("INSERT INTO user SET username='"+escape (user.username)+"', password=PASSWORD ('***')");
	int ret=execute_query ("INSERT INTO user SET username='"+escape (user.username)+"', password=PASSWORD ('"+escape (password)+"')");
	if (ret<0) return ret;
	return sk_user_modify (user);	// To write the other attributes
}

int sk_db::sk_user_modify (const User &user, const QString &username)
	/*
	 * The user "username" (user.username if not given) is modified.
	 * Parameters:
	 *   - user: the data to write.
	 *   - username: optionally the username, this can be used to change the username.
	 * Return value:
	 *   - db_err_* on error.
	 *   - db_ok on success.
	 */
{
	int ret;
	if (!connected ()) return db_err_not_connected;
	if (user.username.isEmpty ()) return db_err_parameter_error;
	QString existing_user_name;
	if (username.isEmpty ())
	{
		// No explicit username given ==> the username from user is used. The
		// user must exist.
		ret=sk_user_exists (user.username);
		if (ret<0) return ret;
		if (ret==0) return db_err_not_found;
		existing_user_name=user.username;
	}
	else
	{
		// An explicit username was given ==> this username is used. The user
		// must exist. The user user.username must *not* exist.
		ret=sk_user_exists (username);
		if (ret<0) return ret;
		if (ret==0) return db_err_not_found;
		ret=sk_user_exists (user.username);
		if (ret<0) return ret;
		if (ret==0) return db_err_already_exists;
		existing_user_name=username;
	}

	QString table_name="user";
	QString condition="username='"+escape (existing_user_name)+"'";
	QString query="update "+table_name+" set "+user_value_list (user)+" where "+condition;

	MYSQL_RES *result;
	ret=execute_query (&result, query, true);
	if (ret==db_ok)
	{
		// Query succeeded
		mysql_free_result (result);
		return db_ok;
	}
	else
	{
		// Query failed
		debug_stream << "There was an error while executing the query in sk_db::write_object (): " << get_last_error () << std::endl;
		return ret;
	}

	return db_ok;
}

int sk_db::sk_user_get (User &user, const QString &username)
	/*
	 * Gets a user from the database.
	 * Parameters:
	 *   - username: the name to get
	 * Parameters set:
	 *   - user: the user found is written there on success.
	 * Return value:
	 *   - db_err_* on error.
	 *   - db_ok on success.
	 */
{
	if (!connected ()) return db_err_not_connected;
	if (username.isEmpty ()) return db_err_parameter_error;

	QList<User> users;
	int ret=sk_user_list (users, username);
	if (ret<0) return ret;

	if (users.empty ()) return db_err_not_found;
	if (users.size ()>1) return db_err_multiple;

	user=users.front ();
	return db_ok;
}

int sk_db::sk_user_delete (const QString &username)
{
	if (!connected ()) return db_err_not_connected;
	if (username.isEmpty ()) return db_err_parameter_error;

	int ret=execute_query ("DELETE FROM user WHERE username='"+escape (username)+"'");
	if (ret<0) return ret;

	return db_ok;
}

int sk_db::sk_user_list (QList<User> &users, const QString &username)
	/*
	 * Returns a user list.
	 * Parameters:
	 *   - username: if not empty, only users with this name are returned.
	 * Parameters set:
	 *   - users: the users are written here.
	 * Return value:
	 *   - db_err_* on error.
	 *   - db_ok on success.
	 */
{
	QString table="user";
	QString condition;
	if (!username.isEmpty ()) condition="username='"+escape (username)+"'";
	QString columns="username,perm_club_admin,perm_read_flight_db,club,person";
	QString condition_string;
	if (!condition.isEmpty ()) condition_string=" where "+condition;
	QString query="select "+columns+" from "+table+condition_string;

	MYSQL_RES *result;
	if (execute_query (&result, query, true)==db_ok)
	{
		// Make a list from the objects
		result_to_user_list (users, result);
		mysql_free_result (result);
		return db_ok;
	}
	else
	{
		return db_err_query_failed;
	}
}

int sk_db::row_to_user (User &user, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields)
	/*
	 * Converts a MySQL row to a user.
	 * Parameters:
	 *   - user: the user to write.
	 *   - row: the row to read from.
	 *   - num_fields: the size of the fields array.
	 *   - fields: the fields in the row.
	 * Return value:
	 *   - db_err_* on error.
	 *   - db_ok on success.
	 */
{
	char *value;
#define USE(key) value=named_field_value (row, num_fields, fields, key);
	USE ("username")            ; user.username=value;
	USE ("perm_club_admin")     ; user.perm_club_admin=(atoi (value)!=0);
	USE ("perm_read_flight_db") ; user.perm_read_flight_db=(atoi (value)!=0);
	USE ("club")                ; user.club=value;
	USE ("person")              ; user.person=atoll (value);
#undef USE

	return db_ok;
}

int sk_db::result_to_user_list (QList<User> &users, MYSQL_RES *result)
	/*
	 * Converts a MySQL result to a list of sk_users.
	 * Parameters:
	 *   - users: the user list to write.
	 *   - result: the result to read.
	 * Return value:
	 *   - db_err_* on error.
	 *   - db_ok on success.
	 */
{
	if (!result) return db_err_parameter_error;

	unsigned int num_fields=mysql_num_fields (result);
	MYSQL_FIELD *fields=mysql_fetch_fields (result);

	// For every row returned...
	MYSQL_ROW row;
	while ((row=mysql_fetch_row (result)))
	{
		User user;

		// Decode the object and add it to the list if successful
		if (row_to_user (user, row, num_fields, fields)==db_ok)
			users.push_back (user);
	}

	return db_ok;
}

QString sk_db::user_value_list (const User &user)
{
	QString r;
	std::ostringstream oss;
#define OUTPUTL(key, value) r+=key; r+="='"; oss.str (""); oss << (value); r+=escape (std2q (oss.str ())); r+="'";
#define OUTPUT(key, value) OUTPUTL (key, value) r+=", ";
	OUTPUT ("username", user.username);
	OUTPUT ("perm_club_admin", user.perm_club_admin?"1":"0");
	OUTPUT ("perm_read_flight_db", user.perm_read_flight_db?"1":"0");
	OUTPUT ("club", user.club);
	OUTPUTL ("person", QString::number (user.person));
#undef OUTPUTL
#undef OUTPUT

	return r;
}




void sk_db::make_flight_data (sk_flug_data &flight_data, const Flight &flight)
	// XXX Ownership is set to true.
{
	// Read the parts of the flight (plane, persons...) from the database.
#define ITEM(TYPE, VAR, GIVEN_CONDITION, GET_FUNCTION, ID, FLIGHT_DATA_MEMBER)	\
	TYPE *VAR=new TYPE;	\
	bool VAR ## _given=GIVEN_CONDITION;	\
	bool VAR ## _ok=false;	\
	int VAR ## _result=db_ok;	\
	if (VAR ## _given)	\
	{	\
		VAR ## _result=GET_FUNCTION (VAR, ID);	\
		VAR ## _ok=(VAR ## _result==db_ok);	\
	}	\
	flight_data.FLIGHT_DATA_MEMBER.set_owner (true);	\
	flight_data.FLIGHT_DATA_MEMBER.set (VAR, VAR ## _given, VAR ## _ok, VAR ## _result);

	ITEM (LaunchType,  sa,       !id_invalid (flight.startart),  get_startart, flight.startart,  startart)
	ITEM (Plane, plane,    !id_invalid (flight.flugzeug),  get_plane,    flight.flugzeug,  plane)
	ITEM (Person,   pilot,    !id_invalid (flight.pilot),     get_person,   flight.pilot,     pilot)
	ITEM (Person,   copilot,  !id_invalid (flight.begleiter), get_person,   flight.begleiter, copilot)
	ITEM (Person,   towpilot, !id_invalid (flight.towpilot),  get_person,   flight.towpilot,  towpilot)
#undef ITEM

	// Towplane is special because we may need to use different
	// retrieval methods.
	// This block is basically the above macro ITEM expanded, apart
	// from the block where the towplane is read from the database.
	// This is not a good solution and TODO.
	Plane *towplane=new Plane;
	bool towplane_given=(sa_ok && sa->is_airtow ());
	bool towplane_ok=false;
	int towplane_result=db_ok;
	if (towplane_given)
	{
		if (sa->towplane_known ())
			towplane_result=get_plane_registration (towplane, sa->get_towplane ());
		else
			towplane_result=get_plane (towplane, flight.towplane);

		towplane_ok=(towplane_result==db_ok);
	}
	flight_data.towplane.set (towplane, towplane_given, towplane_ok, towplane_result);
}


void sk_db::test ()
{
	sk_db db;

//	cout << "Enter root password: ";
//	QString pass=read_password ();
//	db.connect ("localhost", 3306, "root", pass);
//	db.use_db ("startkladdetest");
//	db.sk_user_add ("foo", "bar");
//	return;

	db.connect ("localhost", 3306, "sk_admin", "moobertshausen");
	db.use_db ("startkladdetest");



//	cout << "Exists foo: " << db.sk_user_exists ("foo") << std::endl;
//	cout << "Exists baz: " << db.sk_user_exists ("baz") << std::endl;
//
//	cout << "Auth foo/foo: " << db.sk_user_authenticate ("foo", "foo") << std::endl;
//	cout << "Auth foo/bar: " << db.sk_user_authenticate ("foo", "bar") << std::endl;
//	cout << "Auth baz/baz: " << db.sk_user_authenticate ("baz", "baz") << std::endl;
//	cout << "Auth baz/qux: " << db.sk_user_authenticate ("baz", "qux") << std::endl;

	std::cout << "Get user" << std::endl;
	User user;
	db.sk_user_get (user, "foo");
	std::cout << "Modify user" << std::endl;
	db.sk_user_modify (user);

	db.disconnect ();
	return;
}

