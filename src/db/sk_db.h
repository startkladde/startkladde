#ifndef sk_db_h
#define sk_db_h

#include <iostream>
#include <list>

#include <mysql.h>

// XXX
#include <q3ptrlist.h>
#include <q3valuelist.h>
#define QPtrList Q3PtrList
#define QValueList Q3ValueList
#include <qstringlist.h>

#include "src/accessor.h"
#include "src/condition_t.h"
#include "src/data_types.h"
#include "src/flight_list.h"
#include "src/sk_exception.h"
#include "src/config/options.h"
#include "src/db/db_column.h"
#include "src/db/db_table.h"
#include "src/db/db_types.h"
#include "src/model/sk_flug.h"
#include "src/model/sk_flugzeug.h"
#include "src/model/sk_person.h"
#include "src/model/sk_user.h"
#include "src/model/startart_t.h"

using namespace std;

// TODO replace cerr with debug_stream

// TODO split off startkladde-specifig parts.

// Database function error codes
// TODO error codes as enum?
// TODO exceptions? be careful: make sure mysql_results are freed correctly.
const int db_ok=0;
const int db_err_not_connected=-100;
//const int db_err_allocation_failure=-101;
//const int db_err_connection_error=-102;
const int db_err_too_few_columns=-103;
const int db_err_unhandled=-104;
const int db_err_not_found=-105;
const int db_err_mysql_error=-106;
const int db_err_query_failed=-107;
const int db_err_parameter_error=-108;
const int db_err_not_implemented=-109;
const int db_err_not_editable=-110;
const int db_err_multiple=-111;
const int db_err_already_exists=-112;
const int db_err_not_admin_db=-113;

class sk_db:public QObject
{
	Q_OBJECT

	public:
		// Exceptions
	class ex_allocation_error:public sk_exception/*{{{*/
	{
		public:
			virtual string description () const { return "Speicherfehler"; }
	};
	/*}}}*/
	class ex_parameter_error:public sk_exception/*{{{*/
	{
		public:
			~ex_parameter_error () throw () {};
			ex_parameter_error (const string &_reason) { reason=_reason; }
			virtual string description (bool extended) const
			{
				if (extended && !reason.empty ())
					return description (false)+" ("+reason+")";
				else
					return "Parameterfehler";
			}
			virtual string description () const { return description (false); }
			string reason;
	};
	/*}}}*/
	class ex_connection_failed:public sk_exception/*{{{*/
	{
		public:
			virtual string description () const { return "Verbindungsfehler"; }
	};
	/*}}}*/
	class ex_database_not_accessible:public sk_exception/*{{{*/
	{
		public:
			virtual string description () const { return "Datenbankzugriff nicht m�glich"; }
	};
	/*}}}*/

	// TODO code duplication extended
	class ex_query_failed:public sk_exception/*{{{*/
	{
		public:
			~ex_query_failed () throw () {};
			ex_query_failed (const string &_query) { query=_query; }
			virtual string description (bool extended) const
			{
				if (extended && !query.empty ())
					return description (false)+" ("+query+")";
				else
					return "Query fehlgeschlagen";
			}
			virtual string description () const { return description (false); }
			string query;
	};
	/*}}}*/
	class ex_init_failed:public sk_exception/*{{{*/
	{
		public:
			~ex_init_failed () throw () {};
			ex_init_failed (const string &_reason) { reason=_reason; }
			virtual string description (bool extended) const
			{
				if (extended && !reason.empty ())
					return description (false)+" ("+reason+")";
				else
					return "Datenbankeinrichtung fehlgeschlagen";
			}
			virtual string description () const { return description (false); }
			string reason;
	};
	/*}}}*/
	class ex_operation_failed:public sk_exception/*{{{*/
	{
		public:
			~ex_operation_failed () throw () {};
			ex_operation_failed (const string &_reason) { reason=_reason; }
			virtual string description (bool extended) const
			{
				if (extended && !reason.empty ())
					return description (false)+" ("+reason+")";
				else
					return "Operation fehlgeschlagen";
			}
			virtual string description () const { return description (false); }
			string reason;
	};
	/*}}}*/
	class ex_not_connected:public sk_exception/*{{{*/
	{
		public:
			virtual string description () const { return "Keine Verbindung"; }
	};
	/*}}}*/
	class ex_access_denied:public sk_exception/*{{{*/
	{
		public:
			~ex_access_denied () throw () {};
			ex_access_denied (const string &_message) { message=_message; }
			virtual string description (bool extended) const
			{
				if (extended && !message.empty ())
					return description (false)+" ("+message+")";
				else
					return "Zugriff verweigert";
			}
			virtual string description () const { return description (false); }
			string message;
	};
	/*}}}*/

	class ex_unusable:public sk_exception/*{{{*/
	{
		public:
			virtual string description () const { return "Database unusable"; }
	};
	/*}}}*/
	class ex_table_not_found:public ex_unusable/*{{{*/
	{
		public:
			~ex_table_not_found () throw () {};
			ex_table_not_found (const string &_name) { name=_name; }
			virtual string description () const { return "Tabelle "+name+" nicht gefunden"; }
			string name;
	};
	/*}}}*/
	class ex_column_not_found:public ex_unusable/*{{{*/
	{
		public:
			~ex_column_not_found () throw () {};
			ex_column_not_found (const string &_table, const string &_column) { table=_table; column=_column; }
			virtual string description () const { return "Spalte "+table+":"+column+" nicht gefunden"; }
			string table, column;
	};
	/*}}}*/
	class ex_column_type_mismatch:public ex_unusable/*{{{*/
	{
		public:
			~ex_column_type_mismatch () throw () {};
			ex_column_type_mismatch (const string &_table, const string &_column) { table=_table; column=_column; }
			virtual string description () const { return "Spalte "+table+":"+column+" hat den falschen Typ"; }
			string table, column;
	};
	/*}}}*/
	class ex_insufficient_access:public ex_unusable/*{{{*/
	{
		public:
			~ex_insufficient_access () throw () {};
			ex_insufficient_access (const string &_target) { target=_target; }
			virtual string description () const { return "Ungen�gende Zugriffsrechte auf "+target; }
			string target;
	};
	/*}}}*/
	class ex_database_not_found:public ex_unusable/*{{{*/
	{
		public:
			~ex_database_not_found () throw () {};
			ex_database_not_found (string _name) { name=_name; }
			virtual string description () const { return "Datenbank "+name+" nicht gefunden"; }
			string name;
	};
	/*}}}*/

	class ex_legacy_error:public sk_exception/*{{{*/
		/*
		 * Encapsulates a legacy error code in an exception.
		 */
	{
		public:
			~ex_legacy_error () throw () {};
			ex_legacy_error (int _error, const sk_db &_db): db (_db) { error=_error; }
			virtual string description (bool extended=false) const { return "Datenbankfehler: "+db.db_error_description (error, extended); }
			virtual string description () const { return description (false); }
			int error;
			const sk_db &db;
	};
	/*}}}*/

	class import_message:public sk_exception/*{{{*/
	{
		public:
			// Properties access
			virtual string description (bool extended) const;
			bool fatal () const { return fatal (type); }
			RW_P_ACCESSOR (const sk_person, p1)
			RW_P_ACCESSOR (const sk_person, p2)

#define SINGLE_PERSON_MESSAGE(NAME) \
			static import_message NAME (const sk_person *_p1=NULL, const string &_text_value="") { return import_message (imt_ ## NAME, _p1, NULL, _text_value); }
#define DOUBLE_PERSON_MESSAGE(NAME) \
			static import_message NAME (const sk_person *_p1=NULL, const sk_person *_p2=NULL, const string &_text_value="") { return import_message (imt_ ## NAME, _p1, _p2, _text_value); }

			// Static construction
			SINGLE_PERSON_MESSAGE (first_name_missing)
			SINGLE_PERSON_MESSAGE (last_name_missing)
			DOUBLE_PERSON_MESSAGE (duplicate_club_id)
			DOUBLE_PERSON_MESSAGE (duplicate_name_without_club_id)
			SINGLE_PERSON_MESSAGE (club_id_not_found)
			SINGLE_PERSON_MESSAGE (club_id_old_not_found)
			SINGLE_PERSON_MESSAGE (club_id_not_unique)
			SINGLE_PERSON_MESSAGE (multiple_own_persons_name)
			SINGLE_PERSON_MESSAGE (multiple_own_editable_persons_name)
			SINGLE_PERSON_MESSAGE (multiple_editable_persons_name)
			SINGLE_PERSON_MESSAGE (club_mismatch)
#undef SINGLE_PERSON_MESSAGE
#undef DOUBLE_PERSON_MESSAGE

			// Destruction
			~import_message () throw () {};

		private:
			// Message types
			enum import_message_type
			{
				imt_first_name_missing,
				imt_last_name_missing,
				imt_duplicate_club_id,
				imt_duplicate_name_without_club_id,
				imt_club_id_not_found,
				imt_club_id_old_not_found,
				imt_club_id_not_unique,
				imt_multiple_own_persons_name,
				imt_multiple_own_editable_persons_name,
				imt_multiple_editable_persons_name,
				imt_club_mismatch
			};

			// Constructor
			import_message (import_message_type _type, const sk_person *_p1=NULL, const sk_person *_p2=NULL, const string &_text_value="")
				:type (_type), p1 (_p1), p2 (_p2), text_value (_text_value) {}

			// Properties
			virtual string description () const { return description (false); }
			static bool fatal (import_message_type t);

			// Data members
			import_message_type type;
			const sk_person *p1, *p2;
			string text_value;
	};
/*}}}*/



		// Connection management/*{{{*/
		sk_db (ostream &_debug_stream=cerr);
		~sk_db ();
		void connect (string _server, int _port, string _username, string _password) throw (ex_allocation_error, ex_connection_failed, ex_access_denied);
		void connect () throw (ex_allocation_error, ex_connection_failed, ex_access_denied);
		void use_db (string _database) throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access);
		void use_db () throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access);
		void set_connection_data (string _server, int _port, string _username, string _password);
		void set_database (string _database);
		void set_user_data (string _username, string _password);
		int disconnect ();
		string get_last_error () const;
		unsigned int get_last_errno () const;
		string db_error_description (int error, bool extended=false) const;
		bool connected () const;
		bool alife () const;
		bool silent;
		RO_ACCESSOR (string, last_query)
		RO_ACCESSOR (bool, last_query_success)
		RW_ACCESSOR (bool, is_admin_db)
/*}}}*/

		// Database metainformation/*{{{*/
		void list_required_tables (list<string> &tables);
		void list_required_writeable_tables (list<string> &tables);
		db_table get_table_information (const string table_name, bool resolve_like=false) const;
		void check_usability () throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_not_connected, ex_query_failed, ex_unusable, ex_insufficient_access);
/*}}}*/

		// Database management/*{{{*/
		int grant (string permissions, string target, string user, string password);
		int grant (string permissions, string target, string user);
		int create_database (string name, bool force=false);
		int list_tables (list<string> &tables);
		int create_table (const db_table &tab, bool force=false);
		int list_column_names (list<string> &names, string table);
		int list_columns (list<db_column> &columns, string table);
		int add_column (const string &table, const db_column &column);
		int modify_column (const string &table, const db_column &column);
/*}}}*/

		// Administration
		void merge_person (db_id correct_id, db_id wrong_id) throw (ex_not_connected, ex_parameter_error, ex_query_failed, ex_operation_failed);

		// Importing
		static void remove_editable_persons (QPtrList<sk_person> persons);

		void import_check (const sk_person &person) throw (import_message);
		void import_check (const QPtrList<sk_person> &persons, list<import_message> &notes);

		db_id import_identify (const sk_person &p, list<import_message> *notes=NULL) throw (ex_not_connected, ex_legacy_error, ex_operation_failed, import_message);
		void import_identify (QPtrList<sk_person> &persons, list<import_message> &notes) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);

		db_id import_person (const sk_person &person) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);
		void import_persons (const QPtrList<sk_person> &persons) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);

		// Editable
	public:
		void make_person_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_person, id, editable); }
		void make_flight_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_flight, id, editable); }
		void make_plane_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_plane, id, editable); }

		// Delete/write/check existance/get/count/test/list/*{{{*/
		int delete_flight (db_id id);
		int delete_plane (db_id id);
		int delete_person (db_id id);
		db_id write_flight (sk_flug *p);
		db_id write_plane (sk_flugzeug *p);
		db_id write_person (sk_person *p);
		int flight_exists (db_id id);
		int plane_exists (db_id id);
		int person_exists (db_id id);
		int get_flight (sk_flug *flight, db_id id);
		int get_plane (sk_flugzeug *plane, db_id id);
		int get_person (sk_person *person, db_id id);
		long long int count_flights (condition_t c);
		long long int count_planes (condition_t c);
		long long int count_persons (condition_t c);
		long long int count_flights_current (QDate &date);
		long long int count_flights_today (QDate &date);
		db_id person_flying (db_id id, sk_time_t *given_time);
		db_id plane_flying (db_id id, sk_time_t *given_time);
		bool person_has_flight (db_id id);
		bool person_has_user (db_id id);
		bool plane_has_flight (db_id id);
		bool person_used (db_id id);
		bool plane_used (db_id id);
		int list_flights (QPtrList<sk_flug> &flights, condition_t c);
		int list_planes (QPtrList<sk_flugzeug> &planes, condition_t c);
		int list_persons (QPtrList<sk_person> &persons, condition_t c);
/*}}}*/

		// Listing frontends/*{{{*/
		int list_flights_prepared (QPtrList<sk_flug> &flights);
		int list_flights_date (QPtrList<sk_flug> &flights, QDate *date);
		int list_flights_date_range (flight_list &flights, QDate *start_date, QDate *end_date);
		int list_persons_by_name (QPtrList<sk_person> &persons, string vorname, string nachname);
		int list_persons_by_first_name (QPtrList<sk_person> &persons, string vorname);
		int list_persons_by_last_name (QPtrList<sk_person> &persons, string nachname);
		int list_persons_by_club_club_id (QPtrList<sk_person> &persons, string club, string club_id);
		int list_planes_all (QPtrList<sk_flugzeug> &planes);
		int list_persons_all (QPtrList<sk_person> &persons);
		int list_startarten_all (QPtrList<startart_t> &saen);
		int list_planes_date (QPtrList<sk_flugzeug> &planes, QDate *date);
		int list_persons_date (QPtrList<sk_person> &persons, QDate *date);
		int list_planes_registration (QPtrList<sk_flugzeug> &planes, string registration);
		// TODO get_plane_id_registration oder so
		int get_plane_registration (sk_flugzeug *plane, string registration);
/*}}}*/

		// List strings/*{{{*/
		// TODO UNIQUE
		int list_airfields (QStringList &airfields);
		int list_first_names (QStringList &names, string last_name="");
		int list_last_names (QStringList &names, string first_name="");
		int list_accounting_note (QStringList &notes);
		int list_registrations (QStringList &registrations);
		int list_types (QStringList &types);
		int list_clubs (QStringList &clubs);
/*}}}*/

		// Startarten/*{{{*/
		bool add_startart_to_list (startart_t *sa);
		int get_startart (startart_t *sa, db_id id);
		int get_startart_by_type (startart_t *startart, startart_type sat);
		db_id get_startart_id_by_type (startart_type sat);
		int count_startart ();
/*}}}*/

		int get_towplane (sk_flugzeug *towplane, const startart_t &startart, const db_id towplane_id);

		bool display_queries;
		ostream &debug_stream;

		// Blind queries
		int flush_privileges () { return execute_query ("FLUSH PRIVILEGES"); }

		// Accessors/*{{{*/
		RO_ACCESSOR (string, server);
		RO_ACCESSOR (string, username);
		RO_ACCESSOR (string, password);
		RO_ACCESSOR (string, database);
		RO_ACCESSOR (int, port);
/*}}}*/

		// User management
		int sk_user_exists (const string &username);
		int sk_user_authenticate (const string &username, const string &password);
		int sk_user_change_password (const string &username, const string &password);

		int sk_user_add (const sk_user &user, const string &password);
		int sk_user_modify (const sk_user &user, const string &username="");
		int sk_user_delete (const string &username);

		int sk_user_get (sk_user &user, const string &username);
		int sk_user_list (list<sk_user> &users, const string &username="");

		int row_to_user (sk_user &user, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields);
		int result_to_user_list (list<sk_user> &users, MYSQL_RES *result);
		string user_value_list (const sk_user &user);

		// Data collection
		void make_flight_data (sk_flug_data &flight_data, const sk_flug &flight);


		static void test ();
	private:
		// Connection management/*{{{*/
		MYSQL *mysql;
		bool connection_established;
		string server, username, password, database; int port;
		string current_server;
		string last_query;
		bool last_query_success;
		bool is_admin_db;
/*}}}*/

		// Startart list/*{{{*/
		QPtrList<startart_t> startarten;
/*}}}*/

		// Generic functions/*{{{*/
		string escape (string text);
/*}}}*/

		// Database metainformation/*{{{*/
		string fixed_table_name (db_object_type type);
		string editable_table_name (db_object_type type);
		string default_sort_column (db_object_type type);
/*}}}*/

		// Data type conversion/*{{{*/
		string to_string (sk_time_t *t);
		long long int row_to_number (MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields, const char *field_name);
		void parse (sk_time_t *time, string text);
		string query_column_list (db_object_type otype, bool id_only);
		string query_value_list (db_object_type type, void *object);
		int row_to_object (db_object_type otype, void *object, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields);
/*}}}*/

		// Object management/*{{{*/
		void *new_object (db_object_type type);
		int free_object (db_object_type type, void *object);
		string object_name (db_object_type type, bool plural=false);
		int copy_object (db_object_type type, void *target, void *source);
/*}}}*/

		// Enumeration type conversion/*{{{*/
		static string flugtyp_to_db(flug_typ t);
		static string modus_to_db(flug_modus m);
		static string category_to_db(aircraft_category m);
		static flug_typ db_to_flugtyp (char *in);
		static flug_modus db_to_modus (char *in);
		static aircraft_category db_to_category (char *in);
/*}}}*/

		// Bitmask type conversion/*{{{*/
		static bool status_gestartet (unsigned int status);
		static bool status_gelandet (unsigned int status);
		static bool status_sfz_gelandet (unsigned int status);
		static unsigned int make_status (bool gestartet, bool gelandet, bool sfz_gelandet);
/*}}}*/

		// Query core functions/*{{{*/
		int execute_query (MYSQL_RES **result, string query_string, bool retrieve_immediately);
		int execute_query (string query_string) { return execute_query (NULL, query_string, true); }
		string make_query (query_type_t query_type, db_object_type object_type, string condition, string columns, bool add_column_editable, bool distinct, string sort_column="", string group_column="", bool editable_flag=true);
		string make_condition (condition_t c);
/*}}}*/

		// Direct access helper functions/*{{{*/
		void object_copy (const string &source, const string &target, db_id id) throw (ex_legacy_error);
		void object_delete_from (const string &table, const db_id &id) throw (ex_legacy_error);
		bool object_exists_in (const string &table, const db_id &id) throw (ex_legacy_error);
/*}}}*/


		// Result processing/*{{{*/
		int result_to_list (db_object_type type, QPtrList<void> &result_list, MYSQL_RES *result);
		int result_to_num_list (QValueList<long long int> &num_list, MYSQL_RES *result, const char *field_name);
		int result_to_string_list (QStringList &strings, MYSQL_RES *result, const char *field_name);	// TODO remove
		int result_to_string_list (list<string> &strings, MYSQL_RES *result, const char *field_name);
		int result_to_string_list (list<string> &strings, MYSQL_RES *result, const unsigned int field_num);
		int result_to_id_list (QValueList<db_id> &ids, MYSQL_RES *result);
		char *named_field_value (MYSQL_ROW &row, int num_fields, MYSQL_FIELD *fields, const char *name);
		const char *field_value (const MYSQL_ROW &row, const unsigned int num_fields, MYSQL_FIELD *fields, const unsigned int num);
/*}}}*/

		// Listing/*{{{*/
		int list_id_data (db_object_type type, QValueList<db_id> &ids, QStringList &data_columns, condition_t c);
		int list_ids (db_object_type type, QValueList<db_id> &ids, condition_t c);
		// TODO: custom query only supported for strings
		int list_strings (db_object_type type, string field_name, QStringList &strings, condition_t c, string custom_query="");
		int list_objects (db_object_type type, QPtrList<void> &objects, condition_t c);
		int list_strings_query (const string query, const string field_name, list<string> &strings);
		int list_strings_query (const string query, const unsigned int field_num, list<string> &strings);
/*}}}*/

		// Existance/*{{{*/
		int num_results_query (string query);
		int rows_exist_query (string query);
		int get_object (db_object_type type, void *object, db_id id);
		int object_exists (db_object_type type, db_id id);
/*}}}*/

	// Editable/*{{{*/
	void make_editable (db_object_type type, int id, bool editable) throw (ex_legacy_error, ex_operation_failed);
/*}}}*/

		// Writing, deleting/*{{{*/
		db_id write_object (db_object_type type, void *object);
		int delete_object (db_object_type otype, db_id id);
/*}}}*/

		// Counting/*{{{*/
		long long int count_objects (db_object_type type, condition_t c);
/*}}}*/

		// Frontends/*{{{*/
		db_id object_flying (db_object_type otype, db_id id, sk_time_t *given_time);
		bool object_has_flight (db_object_type otype, db_id id);
/*}}}*/

		// Display alias/*{{{*/
		// This is used for display instead of the next real query, if
		// set. Can be used for hiding passwords from clear text output.
		// Will automatically be reset after the next query.
		void set_query_display_alias (string qda) { query_display_alias=qda; }
		string query_display_alias;
/*}}}*/


	signals:
		void executing_query (string *s);

};

#endif


