#ifndef _Database_h
#define _Database_h

#include <iostream>

#include <mysql.h>

// XXX
#include <q3ptrlist.h>
#include <q3valuelist.h>
#define QPtrList Q3PtrList
#define QValueList Q3ValueList
#include <qstringlist.h>

#include "src/accessor.h"
#include "src/Condition.h"
#include "src/dataTypes.h"
#include "src/FlightList.h"
#include "src/SkException.h"
#include "src/config/Options.h"
#include "src/db/DbColumn.h"
#include "src/db/DbTable.h"
#include "src/db/dbTypes.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/User.h"
#include "src/model/LaunchType.h"

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

class Database:public QObject
{
	Q_OBJECT

	public:
		// Exceptions
	class ex_allocation_error:public SkException
	{
		public:
			virtual QString description () const { return "Speicherfehler"; }
	};

	class ex_parameter_error:public SkException
	{
		public:
			~ex_parameter_error () throw () {};
			ex_parameter_error (const QString &_reason) { reason=_reason; }
			virtual QString description (bool extended) const
			{
				if (extended && !reason.isEmpty ())
					return description (false)+" ("+reason+")";
				else
					return "Parameterfehler";
			}
			virtual QString description () const { return description (false); }
			QString reason;
	};

	class ex_connection_failed:public SkException
	{
		public:
			virtual QString description () const { return "Verbindungsfehler"; }
	};

	class ex_database_not_accessible:public SkException
	{
		public:
			virtual QString description () const { return "Datenbankzugriff nicht m�glich"; }
	};


	// TODO code duplication extended
	class ex_query_failed:public SkException
	{
		public:
			~ex_query_failed () throw () {};
			ex_query_failed (const QString &_query) { query=_query; }
			virtual QString description (bool extended) const
			{
				if (extended && !query.isEmpty ())
					return description (false)+" ("+query+")";
				else
					return "Query fehlgeschlagen";
			}
			virtual QString description () const { return description (false); }
			QString query;
	};

	class ex_init_failed:public SkException
	{
		public:
			~ex_init_failed () throw () {};
			ex_init_failed (const QString &_reason) { reason=_reason; }
			virtual QString description (bool extended) const
			{
				if (extended && !reason.isEmpty ())
					return description (false)+" ("+reason+")";
				else
					return "Datenbankeinrichtung fehlgeschlagen";
			}
			virtual QString description () const { return description (false); }
			QString reason;
	};

	class ex_operation_failed:public SkException
	{
		public:
			~ex_operation_failed () throw () {};
			ex_operation_failed (const QString &_reason) { reason=_reason; }
			virtual QString description (bool extended) const
			{
				if (extended && !reason.isEmpty ())
					return description (false)+" ("+reason+")";
				else
					return "Operation fehlgeschlagen";
			}
			virtual QString description () const { return description (false); }
			QString reason;
	};

	class ex_not_connected:public SkException
	{
		public:
			virtual QString description () const { return "Keine Verbindung"; }
	};

	class ex_access_denied:public SkException
	{
		public:
			~ex_access_denied () throw () {};
			ex_access_denied (const QString &_message) { message=_message; }
			virtual QString description (bool extended) const
			{
				if (extended && !message.isEmpty ())
					return description (false)+" ("+message+")";
				else
					return "Zugriff verweigert";
			}
			virtual QString description () const { return description (false); }
			QString message;
	};


	class ex_unusable:public SkException
	{
		public:
			virtual QString description () const { return "Database unusable"; }
	};

	class ex_table_not_found:public ex_unusable
	{
		public:
			~ex_table_not_found () throw () {};
			ex_table_not_found (const QString &_name) { name=_name; }
			virtual QString description () const { return "Tabelle "+name+" nicht gefunden"; }
			QString name;
	};

	class ex_column_not_found:public ex_unusable
	{
		public:
			~ex_column_not_found () throw () {};
			ex_column_not_found (const QString &_table, const QString &_column) { table=_table; column=_column; }
			virtual QString description () const { return "Spalte "+table+":"+column+" nicht gefunden"; }
			QString table, column;
	};

	class ex_column_type_mismatch:public ex_unusable
	{
		public:
			~ex_column_type_mismatch () throw () {};
			ex_column_type_mismatch (const QString &_table, const QString &_column) { table=_table; column=_column; }
			virtual QString description () const { return "Spalte "+table+":"+column+" hat den falschen Typ"; }
			QString table, column;
	};

	class ex_insufficient_access:public ex_unusable
	{
		public:
			~ex_insufficient_access () throw () {};
			ex_insufficient_access (const QString &_target) { target=_target; }
			virtual QString description () const { return "Ungen�gende Zugriffsrechte auf "+target; }
			QString target;
	};

	class ex_database_not_found:public ex_unusable
	{
		public:
			~ex_database_not_found () throw () {};
			ex_database_not_found (QString _name) { name=_name; }
			virtual QString description () const { return "Datenbank "+name+" nicht gefunden"; }
			QString name;
	};


	class ex_legacy_error:public SkException
		/*
		 * Encapsulates a legacy error code in an exception.
		 */
	{
		public:
			~ex_legacy_error () throw () {};
			ex_legacy_error (int _error, const Database &_db): db (_db) { error=_error; }
			virtual QString description (bool extended=false) const { return "Datenbankfehler: "+db.db_error_description (error, extended); }
			virtual QString description () const { return description (false); }
			int error;
			const Database &db;
	};


	class import_message:public SkException
	{
		public:
			// Properties access
			virtual QString description (bool extended) const;
			bool fatal () const { return fatal (type); }
			RW_P_ACCESSOR (const Person, p1)
			RW_P_ACCESSOR (const Person, p2)

#define SINGLE_PERSON_MESSAGE(NAME) \
			static import_message NAME (const Person *_p1=NULL, const QString &_text_value="") { return import_message (imt_ ## NAME, _p1, NULL, _text_value); }
#define DOUBLE_PERSON_MESSAGE(NAME) \
			static import_message NAME (const Person *_p1=NULL, const Person *_p2=NULL, const QString &_text_value="") { return import_message (imt_ ## NAME, _p1, _p2, _text_value); }

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
			import_message (import_message_type _type, const Person *_p1=NULL, const Person *_p2=NULL, const QString &_text_value="")
				:type (_type), p1 (_p1), p2 (_p2), text_value (_text_value) {}

			// Properties
			virtual QString description () const { return description (false); }
			static bool fatal (import_message_type t);

			// Data members
			import_message_type type;
			const Person *p1, *p2;
			QString text_value;
	};



		// Connection management
		Database (std::ostream &_debug_stream=std::cerr);
		~Database ();
		void connect (QString _server, int _port, QString _username, QString _password) throw (ex_allocation_error, ex_connection_failed, ex_access_denied);
		void connect () throw (ex_allocation_error, ex_connection_failed, ex_access_denied);
		void use_db (QString _database) throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access);
		void use_db () throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access);
		void set_connection_data (QString _server, int _port, QString _username, QString _password);
		void set_database (QString _database);
		void set_user_data (QString _username, QString _password);
		int disconnect ();
		QString get_last_error () const;
		unsigned int get_last_errno () const;
		QString db_error_description (int error, bool extended=false) const;
		bool connected () const;
		bool alife () const;
		bool silent;
		RO_ACCESSOR (QString, last_query)
		RO_ACCESSOR (bool, last_query_success)
		RW_ACCESSOR (bool, is_admin_db)

		// Database metainformation
		void list_required_tables (QStringList &tables);
		void list_required_writeable_tables (QStringList &tables);
		dbTable get_table_information (const QString table_name, bool resolve_like=false) const;
		void check_usability () throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_not_connected, ex_query_failed, ex_unusable, ex_insufficient_access);

		// Database management
		int grant (QString permissions, QString target, QString user, QString password);
		int grant (QString permissions, QString target, QString user);
		int create_database (QString name, bool force=false);
		int list_tables (QStringList &tables);
		int create_table (const dbTable &tab, bool force=false);
		int list_column_names (QStringList &names, QString table);
		int list_columns (QList<DbColumn> &columns, QString table);
		int add_column (const QString &table, const DbColumn &column);
		int modify_column (const QString &table, const DbColumn &column);

		// Administration
		void merge_person (db_id correct_id, db_id wrong_id) throw (ex_not_connected, ex_parameter_error, ex_query_failed, ex_operation_failed);

		// Importing
		static void remove_editable_persons (QPtrList<Person> persons);

		void import_check (const Person &person) throw (import_message);
		void import_check (const QPtrList<Person> &persons, QList<import_message> &notes);

		db_id import_identify (const Person &p, QList<import_message> *notes=NULL) throw (ex_not_connected, ex_legacy_error, ex_operation_failed, import_message);
		void import_identify (QPtrList<Person> &persons, QList<import_message> &notes) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);

		db_id import_person (const Person &person) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);
		void import_persons (const QPtrList<Person> &persons) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);

		// Editable
	public:
		void make_person_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_person, id, editable); }
		void make_flight_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_flight, id, editable); }
		void make_plane_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_plane, id, editable); }

		// Delete/write/check existance/get/count/test/list
		int delete_flight (db_id id);
		int delete_plane (db_id id);
		int delete_person (db_id id);
		db_id write_flight (Flight *p);
		db_id write_plane (Plane *p);
		db_id write_person (Person *p);
		int flight_exists (db_id id);
		int plane_exists (db_id id);
		int person_exists (db_id id);
		int get_flight (Flight *flight, db_id id);
		int get_plane (Plane *plane, db_id id);
		int get_person (Person *person, db_id id);
		long long int count_flights (Condition c);
		long long int count_planes (Condition c);
		long long int count_persons (Condition c);
		long long int count_flights_current (QDate &date);
		long long int count_flights_today (QDate &date);
		db_id person_flying (db_id id, sk_time_t *given_time);
		db_id plane_flying (db_id id, sk_time_t *given_time);
		bool person_has_flight (db_id id);
		bool person_has_user (db_id id);
		bool plane_has_flight (db_id id);
		bool person_used (db_id id);
		bool plane_used (db_id id);
		int list_flights (QPtrList<Flight> &flights, Condition c);
		int list_planes (QPtrList<Plane> &planes, Condition c);
		int list_persons (QPtrList<Person> &persons, Condition c);

		// Listing frontends
		int list_flights_prepared (QPtrList<Flight> &flights);
		int list_flights_date (QPtrList<Flight> &flights, QDate *date);
		int list_flights_date_range (FlightList &flights, QDate *start_date, QDate *end_date);
		int list_persons_by_name (QPtrList<Person> &persons, QString vorname, QString nachname);
		int list_persons_by_first_name (QPtrList<Person> &persons, QString vorname);
		int list_persons_by_last_name (QPtrList<Person> &persons, QString nachname);
		int list_persons_by_club_club_id (QPtrList<Person> &persons, QString club, QString club_id);
		int list_planes_all (QPtrList<Plane> &planes);
		int list_persons_all (QPtrList<Person> &persons);
		int list_startarten_all (QPtrList<LaunchType> &saen);
		int list_planes_date (QPtrList<Plane> &planes, QDate *date);
		int list_persons_date (QPtrList<Person> &persons, QDate *date);
		int list_planes_registration (QPtrList<Plane> &planes, QString registration);
		// TODO get_plane_id_registration oder so
		int get_plane_registration (Plane *plane, QString registration);

		// List strings
		// TODO UNIQUE
		int list_airfields (QStringList &airfields);
		int list_first_names (QStringList &names, QString last_name="");
		int list_last_names (QStringList &names, QString first_name="");
		int list_accounting_note (QStringList &notes);
		int list_registrations (QStringList &registrations);
		int list_types (QStringList &types);
		int list_clubs (QStringList &clubs);

		// Startarten
		bool add_startart_to_list (LaunchType *sa);
		int get_startart (LaunchType *sa, db_id id);
		int get_startart_by_type (LaunchType *startart, startart_type sat);
		db_id get_startart_id_by_type (startart_type sat);
		int count_startart ();

		int get_towplane (Plane *towplane, const LaunchType &startart, const db_id towplane_id);

		bool display_queries;
		std::ostream &debug_stream;

		// Blind queries
		int flush_privileges () { return execute_query ("FLUSH PRIVILEGES"); }

		// Accessors
		RO_ACCESSOR (QString, server);
		RO_ACCESSOR (QString, username);
		RO_ACCESSOR (QString, password);
		RO_ACCESSOR (QString, database);
		RO_ACCESSOR (int, port);

		// User management
		int sk_user_exists (const QString &username);
		int sk_user_authenticate (const QString &username, const QString &password);
		int sk_user_change_password (const QString &username, const QString &password);

		int sk_user_add (const User &user, const QString &password);
		int sk_user_modify (const User &user, const QString &username="");
		int sk_user_delete (const QString &username);

		int sk_user_get (User &user, const QString &username);
		int sk_user_list (QList<User> &users, const QString &username="");

		int row_to_user (User &user, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields);
		int result_to_user_list (QList<User> &users, MYSQL_RES *result);
		QString user_value_list (const User &user);

		// Data collection
		void make_flight_data (sk_flug_data &flight_data, const Flight &flight);


		static void test ();
	private:
		// Connection management
		MYSQL *mysql;
		bool connection_established;
		QString server, username, password, database; int port;
		QString current_server;
		QString last_query;
		bool last_query_success;
		bool is_admin_db;

		// Startart list
		QPtrList<LaunchType> startarten;

		// Generic functions
		QString escape (QString text);

		// Database metainformation
		QString fixed_table_name (db_object_type type);
		QString editable_table_name (db_object_type type);
		QString default_sort_column (db_object_type type);

		// Data type conversion
		QString to_string (sk_time_t *t);
		long long int row_to_number (MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields, const char *field_name);
		void parse (sk_time_t *time, QString text);
		QString query_column_list (db_object_type otype, bool id_only);
		QString query_value_list (db_object_type type, void *object);
		int row_to_object (db_object_type otype, void *object, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields);

		// Object management
		void *new_object (db_object_type type);
		int free_object (db_object_type type, void *object);
		QString object_name (db_object_type type, bool plural=false);
		int copy_object (db_object_type type, void *target, void *source);

		// Enumeration type conversion
		static QString flugtyp_to_db(flug_typ t);
		static QString modus_to_db(flug_modus m);
		static QString category_to_db(aircraft_category m);
		static flug_typ db_to_flugtyp (char *in);
		static flug_modus db_to_modus (char *in);
		static aircraft_category db_to_category (char *in);

		// Bitmask type conversion
		static bool status_gestartet (unsigned int status);
		static bool status_gelandet (unsigned int status);
		static bool status_sfz_gelandet (unsigned int status);
		static unsigned int make_status (bool gestartet, bool gelandet, bool sfz_gelandet);

		// Query core functions
		int execute_query (MYSQL_RES **result, QString query_string, bool retrieve_immediately);
		int execute_query (QString query_string) { return execute_query (NULL, query_string, true); }
		QString make_query (query_type_t query_type, db_object_type object_type, QString condition, QString columns, bool add_column_editable, bool distinct, QString sort_column="", QString group_column="", bool editable_flag=true);
		QString make_condition (Condition c);

		// Direct access helper functions
		void object_copy (const QString &source, const QString &target, db_id id) throw (ex_legacy_error);
		void object_delete_from (const QString &table, const db_id &id) throw (ex_legacy_error);
		bool object_exists_in (const QString &table, const db_id &id) throw (ex_legacy_error);


		// Result processing
		int result_to_list (db_object_type type, QPtrList<void> &result_list, MYSQL_RES *result);
		int result_to_num_list (QValueList<long long int> &num_list, MYSQL_RES *result, const char *field_name);
		int result_to_string_list (QStringList &strings, MYSQL_RES *result, const char *field_name);
		int result_to_string_list (QStringList &strings, MYSQL_RES *result, const unsigned int field_num);
		int result_to_id_list (QValueList<db_id> &ids, MYSQL_RES *result);
		char *named_field_value (MYSQL_ROW &row, int num_fields, MYSQL_FIELD *fields, const char *name);
		const char *field_value (const MYSQL_ROW &row, const unsigned int num_fields, MYSQL_FIELD *fields, const unsigned int num);

		// Listing
		int list_id_data (db_object_type type, QValueList<db_id> &ids, QStringList &data_columns, Condition c);
		int list_ids (db_object_type type, QValueList<db_id> &ids, Condition c);
		// TODO: custom query only supported for strings
		int list_strings (db_object_type type, QString field_name, QStringList &strings, Condition c, QString custom_query="");
		int list_objects (db_object_type type, QPtrList<void> &objects, Condition c);
		int list_strings_query (const QString query, const QString field_name, QStringList &strings);
		int list_strings_query (const QString query, const unsigned int field_num, QStringList &strings);

		// Existance
		int num_results_query (QString query);
		int rows_exist_query (QString query);
		int get_object (db_object_type type, void *object, db_id id);
		int object_exists (db_object_type type, db_id id);

	// Editable
	void make_editable (db_object_type type, int id, bool editable) throw (ex_legacy_error, ex_operation_failed);

		// Writing, deleting
		db_id write_object (db_object_type type, void *object);
		int delete_object (db_object_type otype, db_id id);

		// Counting
		long long int count_objects (db_object_type type, Condition c);

		// Frontends
		db_id object_flying (db_object_type otype, db_id id, sk_time_t *given_time);
		bool object_has_flight (db_object_type otype, db_id id);

		// Display alias
		// This is used for display instead of the next real query, if
		// set. Can be used for hiding passwords from clear text output.
		// Will automatically be reset after the next query.
		void set_query_display_alias (QString qda) { query_display_alias=qda; }
		QString query_display_alias;


	signals:
		void executing_query (QString *s);

};

#endif


