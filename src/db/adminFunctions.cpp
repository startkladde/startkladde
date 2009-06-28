#include "adminFunctions.h"

#include <algorithm>

#include "src/db/DbColumn.h"
#include "src/db/DbTable.h"

void initialize_database (Database &root_db, QString _username, QString _userpass, QString _database, QString _sk_admin_name, QString _sk_admin_password)
	throw (Database::ex_init_failed, Database::ex_not_connected, Database::ex_access_denied, Database::ex_parameter_error)
	// This function initializes the database for use with the program.
	// This assumes that root_db has user data for root and is connected.
	// If the database is half-initialized, it will be finished.
	// TODO: move this to Database?
	// TODO instead of throwing init_failed ("Query fehlgeschlagen"), throw query_failed
{
	// Steps checking the database:
	// # Condition                               check
	// = =========                               =====
	// 1 Connection possible                     mysql_real_connect ()
	// 2 Database access possible                mysql_select_db ()
	// 3 All tables exist                        mysql_list_tables ()
	// 4 Correct tables are writeable            ???
	// 5 All columns exist/have the correct type SHOW COLUMNS FROM

	std::ostream &output=std::cout;

	output << "Initializing database " << _database << " for user " << _username << "." << std::endl;

	// TODO: add reason to the exception returned and add catch-all exception
	// handlers.

	QString query;
	// User database connection for testing.
	Database user_db;
	user_db.set_connection_data (root_db.get_server (), root_db.get_port (), _username, _userpass);
	user_db.set_database (_database);
	user_db.display_queries=root_db.display_queries;

	// sk_admin database connection for testing.
	Database sk_admin_db;
	sk_admin_db.set_connection_data (root_db.get_server (), root_db.get_port (), _sk_admin_name, _sk_admin_password);
	sk_admin_db.set_database (_database);
	sk_admin_db.display_queries=root_db.display_queries;

	// -1. Paramters
	if (_username.isEmpty ()) throw Database::ex_parameter_error ("Benutzername nicht angegeben");
	// Passwords may not be empty.
	if (_userpass.isEmpty ()) throw Database::ex_parameter_error ("Kein Benutzerpasswort angegeben");
	if (_database.isEmpty ()) throw Database::ex_parameter_error ("Datenbank nicht angegeben");
	if (_sk_admin_name.isEmpty ()) throw Database::ex_parameter_error ("Adminstratorname nicht angegeben");
	if (_sk_admin_password.isEmpty ()) throw Database::ex_parameter_error ("Kein Administratorpasswort angegeben");

	// 0. root connection
	// We must already have connected.
	output << "0. Checking root connection..." << std::endl;
	if (!root_db.connected ()) throw Database::ex_not_connected ();

	// Furthermore, we need to get access to the "mysql" database. If we can't
	// get it, that means that we are not root (or something different).
	output << "Opening mysql database" << std::endl;
	try { root_db.use_db ("mysql"); }
	catch (Database::ex_database_not_accessible) { throw Database::ex_init_failed ("root-Datenbankzugriff nicht möglich"); }
	catch (Database::ex_database_not_found) { throw Database::ex_init_failed ("Datenbank \"mysql\" nicht gefunden"); }
	catch (Database::ex_insufficient_access &e) { throw Database::ex_init_failed (e.description ()); }

	// 1a. user connection possible
	// ======================
	// It must be possible for the user to connect to the server.
	// From the MySQL documentation: 'MySQL allows you to create database-level
	// privileges even if the database doesn't exist [...]'
	// So we can simply perform the following query:
	//   - GRANT SELECT ON $database.* TO $username IDENTIFIED BY $userpass
	// This will cause no harm because these permissions are definitely needed
	// and if they are already present, nothing will be changed. If there are
	// already more privileges, this is the user's will and not changed. The
	// password is set to the value specified.
	// Note, however, that we cannot grant the Table specific rights yet
	// because the tables might not exist yet and MySQL does not allow this.

	output << "1a. Granting the user access to the database..." << std::endl;
	if (root_db.grant ("SELECT", _database+".*", _username, _userpass)!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");
	if (root_db.flush_privileges ()!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");

	// Check if we succeeded
	try
	{
		output << "Checking if the user can connect to the server" << std::endl;
		user_db.connect ();
	}
	catch (Database::ex_allocation_error &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_connection_failed &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_access_denied &e)
	{
		// Access still is not possible. This means that something is majorly
		// wrong.
		output << "failed" << std::endl;
		throw Database::ex_init_failed (e.description ());
	}

	// 1b. admin connection possible
	// ======================
	// It must be possible for the admin to connect to the server.
	// The notes from 1a apply her as well.
	// The query performed is:
	//   - grant ALL on $database.* to $admin_name identified by $userpass

	output << "1b. Granting the admin access to the database..." << std::endl;
	if (root_db.grant ("ALL", _database+".*", _sk_admin_name, _sk_admin_password)!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");
	if (root_db.flush_privileges ()!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");

	// Check if we succeeded
	try
	{
		output << "Checking if the admin can connect to the server" << std::endl;
		sk_admin_db.connect ();
	}
	catch (Database::ex_allocation_error &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_connection_failed &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_access_denied &e)
	{
		// Access still is not possible. This means that something is majorly
		// wrong.
		output << "failed" << std::endl;
		throw Database::ex_init_failed (e.description ());
	}

	// 2a. User database access possible
	// =================================
	// It must be possible for the user to access the database (use $database).
	// We gave the user access in step 1. Now we have to create the Table if it
	// does not exist.
	output << "2. Ensuring the database exists..." << std::endl;
	if (root_db.create_database (_database)!=db_ok) throw Database::ex_init_failed ("Fehler beim Anlegen der Datenbank "+_database);

	// Now check if the user can use the database.
	output << "2a. Checking if the user can use the database" << std::endl;
	try { user_db.use_db (); }
	catch (Database::ex_database_not_accessible &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_insufficient_access &e) { throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_database_not_found &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }

	// 2b. Admin database access possible
	// ==================================
	// It must be possible for the admin to access the database (use $database).
	// We created the database in step 2a, so we only have to check.
	output << "2b. Checking if the admin can use the database" << std::endl;
	try { sk_admin_db.use_db (); }
	catch (Database::ex_database_not_accessible &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_insufficient_access &e) { throw Database::ex_init_failed (e.description ()); }
	catch (Database::ex_database_not_found &e) { output << "failed" << std::endl; throw Database::ex_init_failed (e.description ()); }

	// 3. All tables exist
	// There are a number of tables which have to exist for the database to be
	// useful.
	// Check if the required tables to exist. If they don't, create them.
	// For checking, use the user Table as the user must have all require
	// permissions by now.

	output << "3. Checking table existance..." << std::endl;

	// Build a list of tables that are required.
	QStringList tables_required;
	user_db.list_required_tables (tables_required);

	// Get a list of tables that are present.
	QStringList tables_present;
	if (user_db.list_tables (tables_present)!=db_ok) throw Database::ex_init_failed ("Fehler beim Listen der Tabellen");
	QStringList::const_iterator present_end=tables_present.end ();
	QStringList::const_iterator present_begin=tables_present.begin ();

	// Switch to the database
	try
	{
		output << "Switching to the database" << std::endl;
		root_db.use_db (_database);
	}
	catch (...) { throw Database::ex_init_failed ("Unbekannter Fehler (Schritt 3)"); }

	// Create the missing tables
	QStringList::const_iterator required_end=tables_required.end ();
	for (QStringList::const_iterator it=tables_required.begin (); it!=required_end; ++it)
	{
		output << "Checking table " << *it << "...";
		if (std::find (present_begin, present_end, *it)==present_end)
		{
			output << "missing. Creating it." << std::endl;

			dbTable tab=user_db.get_table_information (*it, true);
			if (root_db.create_table (tab)!=db_ok) throw Database::ex_init_failed ("Fehler beim Anlegen der Tabelle "+tab.name);
		}
		else
		{
			output << "OK" << std::endl;
		}
	}

	// 4. Correct tables are writeable
	// ===============================
	// It must be possible for the user to write certain tables.
	// We give the user access unconditionally.
	output << "4. Setting user write access..." << std::endl;
	QStringList writeable;
	user_db.list_required_writeable_tables (writeable);
	if (!writeable.isEmpty ())
	{
		output << "Granting write access to ";
		QStringList::const_iterator end=writeable.end ();
		for (QStringList::const_iterator it=writeable.begin (); it!=end; ++it)
		{
			if (it!=writeable.begin ()) output << ", ";
			output << *it;
		}
		output << " to the user" << std::endl;

		end=writeable.end ();
		for (QStringList::const_iterator it=writeable.begin (); it!=end; ++it)
			if (root_db.grant ("insert,update,delete", _database+"."+*it, _username)!=db_ok)
				throw Database::ex_init_failed ("Fehler bei Query");

		if (root_db.flush_privileges ()!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");
	}
	// ADD: check for success?

	// 5. All columns exist and have the correct type
	// There are some columns which have to exist for the program to be
	// operational.
	// TODO optional columns.

	output << "5. Checking columns..." << std::endl;
	// We can use tables_required/_end from above.
	for (QStringList::const_iterator required_table_name=tables_required.begin (); required_table_name!=required_end; ++required_table_name)
	{
		// required_table_name: iterator to the name of the Table we're checking
		output << "Checking table " << *required_table_name << std::endl;

		// columns_present/_begin/_end: the columns that we have
		QList<DbColumn> columns_present;
		if (user_db.list_columns (columns_present, *required_table_name)!=db_ok) throw Database::ex_init_failed ("Fehler beim Listen der Spalten in "+*required_table_name);
		QList<DbColumn>::const_iterator columns_present_begin=columns_present.begin ();
		QList<DbColumn>::const_iterator columns_present_end=columns_present.end ();

		// columns_required/_end: the columns that we need
		dbTable required_table=user_db.get_table_information (*required_table_name, true);
		QList<DbColumn> &columns_required=required_table.columns;
		QList<DbColumn>::const_iterator columns_required_end=columns_required.end ();

		// Iterate over the required columns, checking existance and correct
		// type.
		for (QList<DbColumn>::const_iterator required_column=columns_required.begin (); required_column!=columns_required_end; required_column++)
		{
			// required_column: iterator to the column we're checking

			// In the list of columns that we have, find the one with the same name.
			QList<DbColumn>::const_iterator col;
			for (col=columns_present_begin; col!=columns_present_end; ++col)
				if ((*col).name==(*required_column).name)
					break;

			// col: iterator to the actually existing column with that name, or
			// columns_present_end if not found.

			if (col==columns_present_end)
			{
				// There is no column with that name
				output << "Column missing: " << (*required_column).name << ". Adding it." << std::endl;

				if (root_db.add_column (*required_table_name, *required_column)!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");
			}
			else if ((*col).type!=(*required_column).type || (*col).length<(*required_column).length)
			{
				// The type does not match or the field is too short
				output << "Column type mismatch: " << (*required_column).name << " is of type " << (*col).type_string () << " but should be " << (*required_column).type_string () << ". Changing it." << std::endl;

				if (root_db.modify_column (*required_table_name, *required_column)!=db_ok) throw Database::ex_init_failed ("Fehler bei Query");
			}
		}
	}

	output << "Database initialization succeeded" << std::endl;
}

void initialize_database (Database &root_db)
	throw (Database::ex_init_failed, Database::ex_access_denied, Database::ex_allocation_error, Database::ex_connection_failed, Database::ex_parameter_error)
	// root_pass is not read from opts but is passed because it usually needs
	// to be asked from the user.
{
	try
	{
		root_db.connect ();
		initialize_database (root_db, opts.username, opts.password, opts.database, opts.sk_admin_name, opts.sk_admin_password);
		root_db.disconnect ();
	}
	// TODO huh?
	catch (Database::ex_not_connected)
	{
		throw Database::ex_connection_failed ();
	}
}

