#include <iostream>

#include "src/version.h"
#include "src/config/options.h"
#include "src/db/admin_functions.h"
#include "src/db/sk_db.h"
#include "src/io/io.h"

void display_help ()/*{{{*/
{
	cout << "usage: skadmin [options...] action [actionopts...]" << endl;
	cout << "  actions:" << endl;
	cout << "    - check_db: check database usability." << endl;
	cout << "    - init_db: initialize the database." << endl;
	cout << "    - noop: no operation (apart from reading the configuration)." << endl;
	cout << "    - merge_person correct_id wrong_id...: merge persons" << endl;
	cout << "  options:" << endl;
	options::display_options ("    ");
}
/*}}}*/

void init_db (sk_db &root_db)/*{{{*/
	// TODO return value
{
	try
	{
		initialize_database (root_db);
	}
	catch (sk_db::ex_init_failed &e)
	{
		cout << e.description (true) << endl;
	}
	catch (sk_exception &e)
	{
		cout << e.description (true) << endl;
	}
}
/*}}}*/

int check_db (sk_db &db)/*{{{*/
	// Returns: 3 for db not usable, 2 for connection/program/server errors
	// That means for doing a db_init: 3: might help, 2: no use
{
	try
	{
		db.connect ();
		db.check_usability ();
		db.disconnect ();
		cout << "Database seems OK" << endl;
	}
	catch (sk_db::ex_access_denied &e) { cout << e.description () << endl; return 3; }
	catch (sk_db::ex_insufficient_access &e) { cout << e.description () << endl; return 3; }
	catch (sk_db::ex_unusable &e) { cout << "Database unusable: " << e.description () << endl; return 3; }
	catch (sk_exception &e) { cout << e.description () << endl; return 2; }
	catch (...) { cout << "Uncaught exception!" << endl; return 2; }

	return 0;
}
/*}}}*/

int merge_person (sk_db &db, const list<string> &args)/*{{{*/
{
	if (args.size ()<2)
	{
		cout << "Error: too few option arguments." << endl
			<< "Usage: merge_person correct_id wrong_id [wrong_id...]" << endl;

		return 1;
	}
	else
	{
		list<string>::const_iterator args_end=args.end ();
		list<string>::const_iterator arg=args.begin ();

		// Determine the correct ID/*{{{*/
		db_id correct_id=atoi ((*arg).c_str ());
		if (id_invalid (correct_id))
		{
			cout << "Error: " << *arg << " is not a valid ID" << endl;
			return 2;
		}
		arg++;
/*}}}*/

		// Determine the wrong IDs/*{{{*/
		list<db_id> wrong_ids;
		for (; arg!=args_end; arg++)
		{
			db_id wrong_id=atoi ((*arg).c_str ());
			if (id_invalid (wrong_id))
			{
				cout << "Error: " << *arg << " is not a valid ID" << endl;
				return 2;
			}
			wrong_ids.push_back (wrong_id);
		}
/*}}}*/

		// Display what we're about to do/*{{{*/
		cout << "Merging wrong person";
		if (wrong_ids.size ()>1) cout << "s";
		for (list<db_id>::iterator i=wrong_ids.begin (); i!=wrong_ids.end (); ++i)
			cout << " " << *i;
		cout << " into " << correct_id << "." << endl;
/*}}}*/

		try
		{
			db.connect ();
			db.use_db ();

			for (list<db_id>::iterator i=wrong_ids.begin (); i!=wrong_ids.end (); ++i)
			{
				db_id wrong_id=*i;
				cout << "Merging " << wrong_id << " into " << correct_id << "." << endl;
				db.merge_person (correct_id, wrong_id);
			}

			cout << "Success" << endl;
		}
		catch (sk_db::ex_operation_failed &e)
		{
			cout << "Error: " << e.description (true) << endl;
			return 2;
		}
		catch (sk_exception &e)
		{
			cout << "Error: " << e.description () << endl;
			return 2;
		}
	}

	return 0;
}
/*}}}*/

int main (int argc, char *argv[])/*{{{*/
{

	if (argc<=1)
	{
		cout << "Error: Missing an action." << endl;
		display_help ();
		return 1;
	}
	else
	{
		if (!opts.parse_arguments (argc, argv))
			return 1;
		else if (opts.need_display ())
			opts.do_display ();
		else if (opts.display_help)
			display_help ();
		else if (opts.non_options.empty ())
		{
			cout << "Error: Missing an action." << endl;
			display_help ();
			return 1;
		}
		else
		{
			enum admin_action { aa_none, aa_init_db, aa_noop, aa_check_db, aa_merge_person };

			string action=opts.non_options.front ();
			admin_action act=aa_none;

			if (action=="init_db") act=aa_init_db;
			else if (action=="check_db") act=aa_check_db;
			else if (action=="noop") act=aa_noop;
			else if (action=="merge_person") act=aa_merge_person;

			if (act!=aa_none) opts.read_config_files (NULL, NULL, argc, argv);

			list<string>::iterator nonopt=opts.non_options.begin (); nonopt++;

			// Determine whether we need an root connection (need_root_db)/*{{{*/
			bool need_root_db=false;
			switch (act)
			{
				case aa_init_db:
				case aa_merge_person:
					need_root_db=true;
					break;
				case aa_check_db:
				case aa_noop:
				case aa_none:
					need_root_db=false;
					break;
			}
/*}}}*/

			// Get the root password (root_password)/*{{{*/
			string root_password;
			if (need_root_db)
			{
				if (opts.root_password.empty ()) 
				{
					cout << "Enter the password for " << opts.root_name << "@" << opts.server_display_name << ":" << opts.port << ": ";
					root_password=read_password ();
				}
				else
				{
					root_password=opts.root_password;
				}
			}
/*}}}*/

			// Set database connection/*{{{*/
			sk_db db;
			db.display_queries=opts.display_queries;
			db.set_database (opts.database);
			if (need_root_db)
				db.set_connection_data (opts.server, opts.port, opts.root_name, root_password);
			else
				db.set_connection_data (opts.server, opts.port, opts.username, opts.password);
/*}}}*/

			switch (act)
			{
				case aa_init_db:
					init_db (db);
					break;
				case aa_check_db:
					return check_db (db);
					break;
				case aa_merge_person:
					return merge_person (db, list<string> (nonopt, opts.non_options.end ()));
					break;
				case aa_noop:
					cout << "noop" << endl;
					break;
				case aa_none:
					cout << "Error: Unknown option " << action << "." << endl;
					display_help ();
					break;
				// No default to allow compiler warnings
			}

			db.disconnect ();
		}
	}

	return 0;
}
/*}}}*/

