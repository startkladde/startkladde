#ifndef DATABASE_H_
#define DATABASE_H_

#include <iostream>
#include <cassert>

#include <mysql.h>

#include <qstringlist.h>

#include "src/accessor.h"
#include "src/Condition.h"
#include "src/SkException.h"
#include "src/config/Options.h"
#include "src/db/DbColumn.h"
#include "src/db/DbTable.h"
#include "src/db/dbTypes.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"

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
const int db_err_timeout=-114;
const int db_err_connection_failed=-115; // TODO is this always due to a connect timeout?

class OldDatabase:public QObject
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
			virtual QString description () const { return "Datenbankzugriff nicht möglich"; }
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

	class ex_database_not_found:public SkException
	{
		public:
			~ex_database_not_found () throw () {};
			ex_database_not_found (QString _name) { name=_name; }
			virtual QString description () const { return "Datenbank "+name+" nicht gefunden"; }
			QString name;
	};

	// TODO handle all timeouts correctly: (a) throw the correct exception at
	// each query/... (b) catch them correctly
	class ex_timeout:public SkException
	{
		public:
			~ex_timeout () throw () {};
			ex_timeout () {};
			virtual QString description () const { return "Timeout"; }
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
			virtual QString description () const { return "Ungenügende Zugriffsrechte auf "+target; }
			QString target;
	};


	class ex_legacy_error:public SkException
		/*
		 * Encapsulates a legacy error code in an exception.
		 */
	{
		public:
			~ex_legacy_error () throw () {};
			ex_legacy_error (int _error, const OldDatabase &_db): db (_db) { error=_error; }
			virtual QString description (bool extended=false) const { return "Datenbankfehler: "+db.db_error_description (error, extended); }
			virtual QString description () const { return description (false); }
			int error;
			const OldDatabase &db;
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



	OldDatabase (std::ostream &_debug_stream=std::cerr);
	~OldDatabase ();


		// Connection management
		void connect (QString server, int port, QString username, QString password) throw (ex_allocation_error, ex_connection_failed, ex_access_denied);
		void use_db (QString database) throw (ex_database_not_accessible, ex_parameter_error, ex_database_not_found, ex_insufficient_access, ex_timeout, ex_connection_failed);
		int disconnect ();
		bool connected () const;
		bool alife () const;
		int ping () const;

		// Errors
		QString get_last_error () const;
		unsigned int get_last_errno () const;
		QString db_error_description (int error, bool extended=false) const;
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
		void merge_person (dbId correct_id, dbId wrong_id) throw (ex_not_connected, ex_parameter_error, ex_query_failed, ex_operation_failed);

		// Importing
		static void remove_editable_persons (QList<Person *> persons);

		void import_check (const Person &person) throw (import_message);
		void import_check (const QList<Person *> &persons, QList<import_message> &notes);

		dbId import_identify (const Person &p, QList<import_message> *notes=NULL) throw (ex_not_connected, ex_legacy_error, ex_operation_failed, import_message);
		void import_identify (QList<Person *> &persons, QList<import_message> &notes) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);

		dbId import_person (const Person &person) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);
		void import_persons (const QList<Person *> &persons) throw (ex_not_connected, ex_legacy_error, ex_operation_failed);

		// Editable
	public:
		void make_person_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_person, id, editable); }
		void make_flight_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_flight, id, editable); }
		void make_plane_editable (int id, bool editable) throw (ex_legacy_error, ex_operation_failed) { make_editable (ot_plane, id, editable); }

		// Delete/write/check existance/get/count/test/list

		// Template functions, specialized in implementation
		template<class T> dbId writeObject (T *);
		template<class T> int getObject (T *, dbId);
		template<class T> int deleteObject (dbId);
		template<class T> bool objectUsed (dbId);


		// TODO make templates
		int flight_exists (dbId id);
		int plane_exists (dbId id);
		int person_exists (dbId id);
		long long int count_flights (Condition c);
		long long int count_planes (Condition c);
		long long int count_persons (Condition c);
		long long int count_flights_current (QDate &date);
		long long int count_flights_today (QDate &date);
		dbId person_flying (dbId id, Time *given_time);
		dbId plane_flying (dbId id, Time *given_time);
		bool person_has_flight (dbId id);
		bool person_has_user (dbId id);
		bool plane_has_flight (dbId id);
		bool person_used (dbId id);
		bool plane_used (dbId id);
		int list_flights (QList<Flight *> &flights, Condition c);
		int list_planes (QList<Plane *> &planes, Condition c);
		int list_persons (QList<Person *> &persons, Condition c);

		// Listing frontends
		int list_flights_prepared (QList<Flight *> &flights);
		int list_flights_date (QList<Flight *> &flights, const QDate *date);
		int list_flights_date_range (QList<Flight *> &flights, QDate *start_date, QDate *end_date);
		int list_persons_by_name (QList<Person *> &persons, QString vorname, QString nachname);
		int list_persons_by_first_name (QList<Person*> &persons, QString vorname);
		int list_persons_by_last_name (QList<Person *> &persons, QString nachname);
		int list_persons_by_club_club_id (QList<Person *> &persons, QString club, QString club_id);
		int list_planes_all (QList<Plane *> &planes);
		int list_persons_all (QList<Person *> &persons);
		int list_startarten_all (QList<LaunchMethod *> &saen);
		int list_planes_date (QList<Plane *> &planes, QDate *date);
		int list_persons_date (QList<Person *> &persons, QDate *date);
		int list_planes_registration (QList<Plane *> &planes, QString registration);
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
		bool add_startart_to_list (LaunchMethod *sa);
		int get_startart (LaunchMethod *sa, dbId id);
		int get_startart_by_type (LaunchMethod *startart, LaunchMethod::Type type);
		dbId get_startart_id_by_type (LaunchMethod::Type type);
		int count_startart ();

		int get_towplane (Plane *towplane, const LaunchMethod &startart, const dbId towplane_id);

		bool display_queries;
		std::ostream &debug_stream;

		// Blind queries
		int flush_privileges () { return execute_query ("FLUSH PRIVILEGES"); }


	private:
		// Connection management
		MYSQL *mysql;
		bool connection_established;
		QString server, username, password, database; int port;
		QString last_query;
		bool last_query_success;
		bool is_admin_db;

		// Startart list
		QList<LaunchMethod *> startarten;

		// Generic functions
		QString escape (QString text);

		// Database metainformation
		QString fixed_table_name (db_object_type type);
		QString editable_table_name (db_object_type type);
		QString default_sort_column (db_object_type type);

		// Data type conversion
		QString to_string (Time *t);
		long long int row_to_number (MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields, const char *field_name);
		void parse (Time *time, QString text);
		QString query_column_list (db_object_type otype, bool id_only);
		QString query_value_list (db_object_type type, void *object);
		int row_to_object (db_object_type otype, void *object, MYSQL_ROW row, int num_fields, MYSQL_FIELD *fields);

		// Object management
		void *new_object (db_object_type type);
		int free_object (db_object_type type, void *object);
		QString object_name (db_object_type type, bool plural=false);
		int copy_object (db_object_type type, void *target, void *source);

		// Enumeration type conversion
		static QString flugtyp_to_db(FlightType t);
		static QString modus_to_db(FlightMode m);
		static QString category_to_db(Plane::Category m);
		static FlightType db_to_flugtyp (char *in);
		static FlightMode db_to_modus (char *in);
		static Plane::Category db_to_category (char *in);

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
		void object_copy (const QString &source, const QString &target, dbId id) throw (ex_legacy_error);
		void object_delete_from (const QString &table, const dbId &id) throw (ex_legacy_error);
		bool object_exists_in (const QString &table, const dbId &id) throw (ex_legacy_error);


		// Result processing
		int result_to_list (db_object_type type, QList<void *> &result_list, MYSQL_RES *result);
		int result_to_num_list (QList<long long int> &num_list, MYSQL_RES *result, const char *field_name);
		int result_to_string_list (QStringList &strings, MYSQL_RES *result, const char *field_name);
		int result_to_string_list (QStringList &strings, MYSQL_RES *result, const unsigned int field_num);
		int result_to_id_list (QList<dbId> &ids, MYSQL_RES *result);
		char *named_field_value (MYSQL_ROW &row, int num_fields, MYSQL_FIELD *fields, const char *name);
		const char *field_value (const MYSQL_ROW &row, const unsigned int num_fields, MYSQL_FIELD *fields, const unsigned int num);

		// Listing
		int list_id_data (db_object_type type, QList<dbId> &ids, QStringList &data_columns, Condition c);
		int list_ids (db_object_type type, QList<dbId> &ids, Condition c);
		// TODO: custom query only supported for strings
		int list_strings (db_object_type type, QString field_name, QStringList &strings, Condition c, QString custom_query="");
		int list_objects (db_object_type type, QList<void *> &objects, Condition c);
		int list_strings_query (const QString query, const QString field_name, QStringList &strings);
		int list_strings_query (const QString query, const unsigned int field_num, QStringList &strings);

		// Existance
		int num_results_query (QString query);
		int rows_exist_query (QString query);
		int get_object (db_object_type type, void *object, dbId id);
		int object_exists (db_object_type type, dbId id);

	// Editable
	void make_editable (db_object_type type, int id, bool editable) throw (ex_legacy_error, ex_operation_failed);

		// Writing, deleting
		dbId write_object (db_object_type type, void *object);
		int delete_object (db_object_type otype, dbId id);

		// Counting
		long long int count_objects (db_object_type type, Condition c);

		// Frontends
		dbId object_flying (db_object_type otype, dbId id, Time *given_time);
		bool object_has_flight (db_object_type otype, dbId id);

		// Display alias
		// This is used for display instead of the next real query, if
		// set. Can be used for hiding passwords from clear text output.
		// Will automatically be reset after the next query.
		void set_query_display_alias (QString qda) { query_display_alias=qda; }
		QString query_display_alias;


	signals:
		void executing_query (QString s);

};

#endif


