#include <string>
#include <iostream>
#include <fstream>
#include "src/version.h"
#include "src/accessor.h"
#include "src/web/argument.h"
#include "src/text.h"
#include "src/web/web_session.h"
#include "src/db/sk_db.h"
#include "src/io/io.h"
#include "src/config/options.h"

// Qt3:
//#include <qptrlist.h>
// Qt4:
#include <Qt3Support>
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator

#include "src/web/html_document.h"
#include "src/web/http_document.h"
#include "src/web/what_next.h"
#include "src/model/sk_user.h"
#include "src/object_field.h"
#include "src/web/mime_header.h"
#include "src/documents/table.h"
#include "src/time/sk_date.h"
#include "src/statistics/flugbuch.h"
#include "src/statistics/bordbuch.h"
#include "src/documents/latex_document.h"
#include "src/plugins/plugin_data_format.h"

// Places that need to be changed when adding a web interface state are marked
// with _STATE_

// TODO: check: fehlermeldung hat korrekten Titel (vorheriger state)

// TODO mehr write_paragraph
// TODO mehr write_text

// TODO: check all parameters from the user for validity
//   - passw�rter: d�rfen keine "" enthalten (?)

// Person editieren: +Bemerkung +Vereinsid +L�schen

// TODO: When changing to the same state (login->do_login->login), preenter the
// values like username (not password), like already done for some states.

// TODO check html_escape

// Warum funktioniert Location: nur mit absoluter URL? Mit relativer wird, so
// scheint mir, vom Browser zwar die angegebene Seite geladen (keine
// Dokumentausgabe vorhanden, trotzdem etwas geladen), aber die alte URL wird
// noch angezeigt.

/*
 * Design decisions:
 *   - For getting information from one state to another, don't use
 *     the session if there is another way because session variables
 *     are something like (session) global variables whereas other
 *     variables are local to the page view.
 *
 */

/*
 * Diagnostics:
 *   - "session_user nicht gesetzt": This probably means that in the
 *   description for a state, the required access level ist set to
 *   dba_none and in the handler the user is accessed, for example via
 *   require_club_admin.
 */

/*
 * Mechanism for getting information from one state to another:
 *   - Via session
 *     - add a session variable arg_session_(new_state)_(what_you_like)
 *     - in the original state, use SESSION_WRITE to store the information in
 *       the session.
 *     - in the new state, use SESSION_READ to read the information
 *     - in cleanup_session, delete the values.
 *   - one_time_message
 *     This is a special case of "via session". Set the session
 *     argument arg_session_one_time_message (SESSION_WRITE
 *     (one_time_message, "...")). The message is displayed on the
 *     next call of the program. Then, the message is deleted from the
 *     session. That means that when the page is reloaded, the message
 *     is no longer displayed, which is good for reporting success.
 *     TODO is it really deleted when outputting an error?
 */

/*
 * State change mechanisms
 *   - return what_next::go_to_state (new_state). This changes the state
 *     immediately. Any output written so far is discarded.
 *     You can also pass a message to be displayed.
 *   - Write a back link (back_link_url). This allows the user to enter a
 *     state. The new state is entered in another program run.
 *   - Write a redirect. This enters the new state in another program run
 *     without any user intervention. This can be used to avoid reposting the
 *     data that leaded to an output page when the user reloads the page, for
 *     example after changing the password. It can also be used to add the
 *     parameters to the address after a POST. For example, after login, a
 *     redirect to the main menu is done.
 */

/*
 * Rules for writing state handlers:
 *   - Before entering a session state (either via link in
 *     output_document or via go_to_state) from a non-session state, make sure
 *     a session exists (session.is_ok ()).
 *   - After a POST, there must be a redirect to add the headers, so it is not
 *     POSTed again when the user reloads the page.
 *   - Every state must either output data, output an error, change to
 *     another state or do a redirect.
 *   - It can be assumed that, upon entrance to the state handler, the database
 *     is properly connected if specified so in the state description. Likewise,
 *     it can be assumed that a session exists if specified so.
 *   - If there is a session, session_access and session_username are set
 *     correctly. If session_access==dba_sk_user, session_user is also set.
 *     Note that when a state requires dba_sk_user, it can also be called with
 *     superior permissions, like dba_sk_admin. In this case, there is no
 *     session user. So before using session_user, always check if
 *     session_access==dba_sk_user.
 *   - For helper functions returning what_next, use the DO_SUB_ACTION macro.
 *   - A state that is called with POST, enctype=form/multipart must save the
 *     files passed via the session and do a redirect immediately.
 */



/*****************************
 * ALL NEW WEBINTERFACE CODE *
 *****************************/

// TODO this is specific code.
// Actually, the very fact that we have a database is specific code. Need to do
// something about this.

//   - dba_none: no database access is done
//   - dba_access: access is done, but no specific authentication level is required
//   - dba_sk_user: a sk user must be authenticated
//   - dba_sk_admin: the sk admin mysql user must be logged in
//   - dba_root: the root mysql user must be logged in
//   TODO this is not sufficient because the state init_db requires root, but
//   does not require the database to be selected (in fact, this is not even
//   possible).

// TODO replace this because it mixes authentication and db access/authorization.
enum db_access_t/*{{{*/
{
	dba_none,
	dba_access,
	dba_sk_user,
	dba_sk_admin,
	dba_root
};
/*}}}*/

string db_access_string (db_access_t a)/*{{{*/
	/*
	 * Makes a description for a db_access_t.
	 * Paramters:
	 *   - a: the access to get the description for.
	 * Return value:
	 *   - the description
	 */
{
	switch (a)
	{
		case dba_none: return "dba_none";
		case dba_access: return "dba_access";
		case dba_sk_user: return "dba_sk_user";
		case dba_sk_admin: return "dba_sk_admin";
		case dba_root: return "dba_root";
	}

	return "???";
}
/*}}}*/

bool provides (db_access_t a, db_access_t b)/*{{{*/
	/*
	 * Returns whether one db access levels provides another.
	 * Parameters:
	 *   - a, b: two access levels.
	 * Return value:
	 *   true if a provides b, that is, if everything that can be done with b
	 *   can also be done with a.
	 */
{
#define HANDLE_A(A_CASE, PRO_none, PRO_access, PRO_sk_user, PRO_sk_admin, PRO_root)	\
	case A_CASE:	\
		HANDLE_B (PRO_none, PRO_access, PRO_sk_user, PRO_sk_admin, PRO_root)	\
		break;

#define HANDLE_B(PRO_none, PRO_access, PRO_sk_user, PRO_sk_admin, PRO_root)	\
	switch (b)	\
	{	\
		case dba_none: return PRO_none; break;	\
		case dba_access: return PRO_access; break;	\
		case dba_sk_user: return PRO_sk_user; break;	\
		case dba_sk_admin: return PRO_sk_admin; break;	\
		case dba_root: return PRO_root; break;	\
	}	\

	switch (a)
	{
		//        A    provides none   access, sk_user sk_admin root
		HANDLE_A (dba_none,     true , false,  false,  false,   false)
		HANDLE_A (dba_access,   true , true ,  false,  false,   false)
		HANDLE_A (dba_sk_user,  true , true ,  true ,  false,   false)
		HANDLE_A (dba_sk_admin, true , true ,  true ,  true ,   false)
		HANDLE_A (dba_root,     true , true ,  true ,  true ,   true )
	}
#undef HANDLE_B
#undef HANDLE_A

	// Now this is an error, actually.
	cerr << "Unhandled db_access_t case in provides ()" << endl;
	return false;
}
/*}}}*/


class web_interface_state;
class what_next;
// TODO consider replacing state_hander by function objects
typedef what_next (*state_handler) ();

class web_interface_state/*{{{*/
{
	public:
		class ex_not_found: public exception {};

		web_interface_state (
				const string &_label,
				bool _has_output,
				db_access_t _db_access_needed,
				bool _allow_anon,
				bool _allow_local,
				const string &_caption,
				const state_handler &_handler
				):
			label (_label),
			has_output (_has_output),
			db_access_needed (_db_access_needed),
			allow_anon (_allow_anon),
			allow_local (_allow_local),
			caption (_caption),
			handler (_handler)
			{}

		web_interface_state ():
			has_output (false),
			db_access_needed (dba_none),
			allow_anon (false),
			allow_local (false),
			caption ("empty"),
			handler (NULL)
			{}

		static bool add_to_list (const web_interface_state &state);
		static const web_interface_state &from_list (const string &_label) throw (ex_not_found);

		string make_caption () const { return caption; }

		RO_ACCESSOR (string, label)
		RO_ACCESSOR (bool, has_output)
		RO_ACCESSOR (db_access_t, db_access_needed)
		RO_ACCESSOR (bool, allow_anon)
		RO_ACCESSOR (bool, allow_local)
		RO_ACCESSOR (state_handler, handler)

	private:
		static list<web_interface_state> state_list;

		// Variables
		// Identification
		string label;

		// Properties
		bool has_output;
		db_access_t db_access_needed;
		// TODO replace access control scheme
		bool allow_anon;    // Whether anonymous access to this state is allowed
							// (i. e., everyone, even from the web, even if not
							// authenticated). Needed at least for logging in.
		bool allow_local;   // Whether to allow allow anonymous access from
		                    // local hosts.
		string caption;
		state_handler handler;
};
/*}}}*/

// web_interface_state static variables
list<web_interface_state> web_interface_state::state_list;

bool web_interface_state::add_to_list (const web_interface_state &state)/*{{{*/
	// Only, if label is unique
{
	// TODO hat std::list nicht so was wie find_if?
	list<web_interface_state>::const_iterator end=state_list.end ();
	for (list<web_interface_state>::const_iterator it=state_list.begin (); it!=end; ++it)
		if ((*it).label==state.label)
			return false;

	state_list.push_back (state);
	return true;
}
/*}}}*/

const web_interface_state &web_interface_state::from_list (const string &_label)/*{{{*/
	throw (ex_not_found)
{
	// TODO hat std::list nicht so was wie find_if?
	list<web_interface_state>::const_iterator end=state_list.end ();
	for (list<web_interface_state>::const_iterator it=state_list.begin (); it!=end; ++it)
		if ((*it).label==_label)
			return *it;

	throw ex_not_found ();
}
/*}}}*/


class ex_write_error_document: public exception/*{{{*/
{
	public:
		ex_write_error_document (const string &_message):message (_message), program_error (false) {}
		~ex_write_error_document () throw () {}
		ex_write_error_document &is_program_error (bool _program_error=true) { program_error=_program_error; return *this; }
		string message;
		bool program_error;
};
/*}}}*/

class ex_go_to_state: public exception/*{{{*/
{
	public:
		ex_go_to_state (const string &_state):state (_state), error (false) {}
		~ex_go_to_state () throw () {}
		ex_go_to_state &with_message (const string &_message) { message=_message; error=false; return *this; }
		ex_go_to_state &with_error_message (const string &_message) { message=_message; error=true; return *this; }
		string state;
		string message;
		bool error;
};
/*}}}*/

/****************************
 * SPECIFIC CODE BELOW HERE *
 ****************************/

// Data types
enum user_class_t/*{{{*/
	/*
	 * Users of the web interface can be categorized in different classes. This
	 * determines how they are authenticated and authorized.
	 */
{
	uc_none,	// Not logged in/not allowed to log in
	uc_sk_user,	// sk user, no correponding MySQL user exists, MySQL login as
				// sk_admin
	uc_mysql_user	// MySQL user, log in as such
};
/*}}}*/

// States/*{{{*/
	/*
	 * The web interface is modelled as a FSM. It has different states, each of
	 * which peforms an action an then outputs a result page or goes to another
	 * state.
	 * States are identified by a string rather than an enum value because this
	 * avoids lots of looking up of states as states need to be identified by
	 * strings in the web interface (an enum was used before and it was not
	 * very good).
	 */
// _STATE_
const string web_login                 ="login";
const string web_do_login              ="do_login";
const string web_main_menu             ="main_menu";
const string web_logout                ="logout";
const string web_change_password       ="change_password";
const string web_do_change_password    ="do_change_password";
const string web_list_persons          ="list_persons";
const string web_edit_person           ="edit_person";
const string web_do_edit_person        ="do_edit_person";
const string web_result                ="result";
const string web_display_person        ="display_person";
const string web_delete_person         ="delete_person";
const string web_do_delete_person      ="do_delete_person";
const string web_create_person         ="create_person";
const string web_do_create_person      ="do_create_person";
const string web_select_merge_person   ="select_merge_person";
const string web_merge_person          ="merge_person";
const string web_do_merge_person       ="do_merge_person";

const string web_user_list                ="user_list";
const string web_user_delete              ="user_delete";
const string web_user_do_delete           ="user_do_delete";
const string web_user_add                 ="user_add";
const string web_user_edit                ="user_edit";
const string web_user_do_edit             ="user_do_edit";
const string web_user_change_password     ="user_change_password";
const string web_user_do_change_password  ="user_do_change_password";
const string web_person_select            ="person_select";

const string web_master_data_import    ="master_data_import";
const string web_master_data_upload    ="master_data_upload";
const string web_master_data_check     ="master_data_check";
const string web_master_data_do_import ="master_data_do_import";

const string web_person_logbook    ="person_logbook";
const string web_do_person_logbook ="do_person_logbook";
const string web_plane_logbook     ="plane_logbook";
const string web_do_plane_logbook  ="do_plane_logbook";
const string web_flightlist        ="flightlist";
const string web_do_flightlist     ="do_flightlist";
const string web_flight_db         ="flight_db";
const string web_do_flight_db      ="do_flight_db";

const string web_test_redirect ="test_redirect";

// TODO: do_add und do_edit zusammenfassen?
// TODO: user_change_pass und change_password Zusammenfassen?

const string &default_state=web_main_menu;
/*}}}*/

enum treatment_t/*{{{*/
	/*
	 * "Treating" an object is performing a certain action, like displaying or
	 * editing.  Treating is obsoleted by the object_field mechanism and only
	 * used for persons.
	 */
{
	tm_write_table_prefix,
	tm_write_table_suffix,
	tm_write_table_header,
	tm_write_table_data,
	tm_write_display,
	tm_write_edit,
	tm_write_create,
	tm_edit,
	tm_create
};
/*}}}*/

bool object_required (treatment_t t)/*{{{*/
	/*
	 * Whether we need a object to do the action.
	 * For example, writing a table header does not require this.
	 */
{
	switch (t)
	{
		case tm_write_table_prefix: return false;
		case tm_write_table_suffix: return false;
		case tm_write_table_header: return false;
		case tm_write_table_data: return false;
		case tm_write_display: return true;
		case tm_write_edit: return true;
		case tm_write_create: return false;
		case tm_edit: return true;
		case tm_create: return true;
	}
	return true;
}
/*}}}*/

enum request_method_t/*{{{*/
{
	rm_post,
	rm_get,
	rm_commandline,
	rm_other
};
/*}}}*/


// Constants/Variables
// CSV fields/*{{{*/
// Column names for CSV columns.
const string csv_field_person_last_name="Nachname";
const string csv_field_person_first_name="Vorname";
const string csv_field_person_comments="Bemerkungen";
const string csv_field_person_club_id="Vereins-ID";
const string csv_field_person_club_id_old="Vereins-ID_alt";
/*}}}*/

// Arguments/*{{{*/
/*
 * Arguments can be stored in the CGI query and in the session. They are
 * identified by labels which have symbolic names defined here.
 */
// These are arguments used by the CGI.
const string arg_cgi_session_id="session_id";
const string arg_cgi_new_state="action";
const string arg_cgi_debug="debug";
const string arg_cgi_url="url";
const string arg_cgi_no_redirects="no_redirects";
const string arg_cgi_username="username";
const string arg_cgi_password="password";
const string arg_cgi_old_password="old_password";
const string arg_cgi_new_password_1="new_password_1";
const string arg_cgi_new_password_2="new_password_2";
const string arg_cgi_last_name="last_name";
const string arg_cgi_first_name="first_name";
const string arg_cgi_club="club";
const string arg_cgi_id="id";
const string arg_cgi_club_id="club_id";
const string arg_cgi_comments="comments";
const string arg_cgi_editable="editable";
const string arg_cgi_club_admin="club_admin";
const string arg_cgi_create_new="create_new";
const string arg_cgi_select_person="select_person";
const string arg_cgi_user_person="user_person";
const string arg_cgi_correct_person="correct_person";
const string arg_cgi_data_type="data_type";
const string arg_cgi_file="file";
const string arg_cgi_format="format";
const string arg_cgi_date_spec="date_spec";
const string arg_cgi_flight_instructor_mode="flight_instructor_mode";
const string arg_cgi_date_single_year="date_single_year";
const string arg_cgi_date_single_month="date_single_month";
const string arg_cgi_date_single_day="date_single_day";
const string arg_cgi_date_start_year="date_start_year";
const string arg_cgi_date_start_month="date_start_month";
const string arg_cgi_date_start_day="date_start_day";
const string arg_cgi_date_end_year="date_end_year";
const string arg_cgi_date_end_month="date_end_month";
const string arg_cgi_date_end_day="date_end_day";
const string arg_cgi_data_format="data_format";


// Values
const string arg_cgi_data_type_person="person";
const string arg_cgi_format_html="html";
const string arg_cgi_format_csv="csv";
const string arg_cgi_format_pdf="pdf";
const string arg_cgi_format_latex="latex";
const string arg_cgi_date_spec_today="today";
const string arg_cgi_date_spec_single="single";
const string arg_cgi_date_spec_range="range";
const string arg_cgi_flight_instructor_mode_no="no";
const string arg_cgi_flight_instructor_mode_strict="strict";
const string arg_cgi_flight_instructor_mode_loose="loose";
const string arg_cgi_towflights_extra="towflights_extra";
const string arg_cgi_data_format_default="default";


// These are arguments stored in the session.
const string arg_session_remote_address="remote_address";
const string arg_session_login_name="login_name";
const string arg_session_password="password";
const string arg_session_result_text="result_text";
const string arg_session_result_error="result_error";
const string arg_session_result_state="result_state";
const string arg_session_one_time_message="one_time_message";
const string arg_session_create_new="create_new";
const string arg_session_master_data_file="file";
const string arg_session_master_data_filename="filename";

// These are arguments used (usually in the CGI query) for setting fields of the
// object_field mechanism.
const string field_user_username="f_user_username";
const string field_user_password="f_user_password";
const string field_user_password_repeat="f_user_password_repeat";
const string field_user_club_admin="f_user_club_admin";
const string field_user_read_flight_db="f_user_read_flight_db";
const string field_user_club="f_user_club";
const string field_user_person="f_user_person";

const string field_flugbuch_tag             ="tag";
const string field_flugbuch_muster          ="muster";
const string field_flugbuch_kennzeichen     ="kennzeichen";
const string field_flugbuch_flugzeugfuehrer ="flugzeugfuehrer";
const string field_flugbuch_begleiter       ="begleiter";
const string field_flugbuch_startart        ="startart";
const string field_flugbuch_ort_start       ="ort_start";
const string field_flugbuch_ort_landung     ="ort_landung";
const string field_flugbuch_zeit_start      ="zeit_start";
const string field_flugbuch_zeit_landung    ="zeit_landung";
const string field_flugbuch_flugdauer       ="flugdauer";
const string field_flugbuch_bemerkung       ="bemerkung";

const string field_bordbuch_date         ="date";
const string field_bordbuch_club         ="club";
const string field_bordbuch_registration ="registration";
const string field_bordbuch_plane_type   ="plane_type";
const string field_bordbuch_name         ="name";
const string field_bordbuch_num_persons  ="num_persons";
const string field_bordbuch_place_from   ="place_from";
const string field_bordbuch_place_to     ="place_to";
const string field_bordbuch_starttime    ="starttime";
const string field_bordbuch_landtime     ="landtime";
const string field_bordbuch_num_landings ="num_landings";
const string field_bordbuch_flight_time  ="flight_time";

const string field_flight_number            ="number";
const string field_flight_registration      ="registration";
const string field_flight_type              ="type";
const string field_flight_pilot             ="pilot";
const string field_flight_copilot           ="copilot";
const string field_flight_club              ="club";
const string field_flight_startart          ="startart";
const string field_flight_starttime         ="starttime";
const string field_flight_landtime          ="landtime";
const string field_flight_duration          ="duration";
const string field_flight_landtime_towplane ="landtime_towplane";
const string field_flight_duration_towplane ="duration_towplane";
const string field_flight_num_landings      ="num_landings";
const string field_flight_startort          ="startort";
const string field_flight_zielort           ="zielort";
const string field_flight_zielort_towplane  ="zielort_towplane";
const string field_flight_comments          ="comments";
// Additional fields not used for flight lists, only for the flight db.
const string field_flight_date                     ="date";
const string field_flight_plane_club               ="plane_club";
const string field_flight_pilot_last_name          ="pilot_last_name";
const string field_flight_pilot_first_name         ="pilot_first_name";
const string field_flight_pilot_club               ="pilot_club";
const string field_flight_pilot_club_id            ="pilot_club_id";
const string field_flight_copilot_last_name        ="copilot_last_name";
const string field_flight_copilot_first_name       ="copilot_first_name";
const string field_flight_copilot_club             ="copilot_club";
const string field_flight_copilot_club_id          ="copilot_club_id";
const string field_flight_flight_type              ="flight_type";
const string field_flight_mode                     ="mode";
const string field_flight_registration_towplane    ="registration_towplane";
const string field_flight_mode_towplane            ="mode_towplane";
const string field_flight_abrechnungshinweis       ="abrechnungshinweis";
const string field_flight_id                       ="id";
/*}}}*/


// Global (semi-)constants/*{{{*/
const string caption_prefix="Startkladde Web-Administration: ";
string relative_url;				// The relative URL where to reach the cgi program
string absolute_url;				// The absolute URL where to reach the cgi program
string remote_address;				// The address of the client
request_method_t request_method;    // The request method
// TODO need to implement object_field successor. These global vars
// mixing data and metadata are MURX.
list<object_field> fields_sk_user;	        // The member field description of an sk_user
list<object_field> fields_flugbuch_entry;	// The member field description of a flugbuch_entry
list<object_field> fields_bordbuch_entry;	// The member field description of a bordbuch_entry
list<object_field> fields_flightlist_entry; // The member field description of a flight list entry
list<object_field> fields_flight_db_entry;  // The member field description of a flight list entry
list<float> widths_flugbuch_entry;			// The column widths of a flugbuch entry, in mm
list<float> widths_bordbuch_entry;			// The column widths of an bordbuch entry, in mm
list<float> widths_flightlist;			    // The column widths of a flightlist entry, in mm
/*}}}*/

// Global variables/*{{{*/
bool debug_enabled;                         // Whether debugging is enabled
ostringstream debug_stream;                 // A stream whose contents are
											// displayed in the document footer
											// when the debug argument is set
web_session session;                        // The login session
argument_list cgi_args;                     // Arguments passed via CGI
argument_list filenames;			// Mapping from CGI parameters to filenames
sk_db db (debug_stream);                    // The database connection
html_document document (true);              // The document to write (to)
const web_interface_state *current_state;   // The state currently executing
db_access_t session_access;                 // The access level that is present
                                            // for the session.
string session_username;                    // The name of the web interface
                                            // user logged in.
const sk_user *session_user;                // The user logged in if
                                            // session_access==dba_sk_user
string query_string;                        // The CGI query string (might not
											// exist for form/multipart POSTs)
bool client_is_local;
/*}}}*/


// Helper functions
// Without _F: arg_cgi_ or arg_session_ is prepended to the variable name
#define SESSION_READ(VAR) session.args.get_value (arg_session_ ## VAR)
#define SESSION_HAS(VAR) session.args.has_argument (arg_session_ ## VAR)
#define SESSION_WRITE(VAR, TEXT) session.args.set_value (arg_session_ ## VAR, TEXT)
#define SESSION_REMOVE(VAR) session.args.remove (arg_session_ ## VAR)

// TODO: add a default ("not found" value)
#define CGI_READ(VAR) cgi_args.get_value (arg_cgi_ ## VAR)
#define CGI_HAS(VAR) cgi_args.has_argument (arg_cgi_ ## VAR)
#define CGI_WRITE(VAR, TEXT) cgi_args.set_value (arg_cgi_ ## VAR, TEXT)

#define CGI_READ_F(VAR) cgi_args.get_value (VAR)
#define CGI_HAS_F(VAR) cgi_args.has_argument (VAR)


// Generic functions
// TODO move to text
const string &message_or_default (const string &message, const string &def)/*{{{*/
	/*
	 * Returns a message, or, if it is empty, a default message.
	 * Parameters:
	 *   - message: the message to return, if not empty
	 *   - def: the default message
	 * Return value:
	 *   - message if it is not empty
	 *   - def else
	 */
{
	if (message.empty ())
		return def;
	else
		return message;
}
/*}}}*/

sk_date date_from_cgi (const string &year_arg, const string &month_arg, const string &day_arg)/*{{{*/
	/*
	 * Reads a date from the CGI parameters:
	 * Parameters:
	 *   - year_arg, month_arg, day_arg: the names of the parameters containing
	 *     the values for the date componentes.
	 * Return value:
	 *   The date read from the CGI parameters.
	 */
{
	unsigned int year=atoi (cgi_args.get_value (year_arg).c_str ());
	unsigned int month=atoi (cgi_args.get_value (month_arg).c_str ());
	unsigned int day=atoi (cgi_args.get_value (day_arg).c_str ());

	// If one parameter is not given, this date is automatically invalid as the
	// value will be 0.
	return sk_date (year, month, day);
}
/*}}}*/


// Requirements checking
// Common parameters:
//   - message: a message to output instead of the default message in case the
//     requirement is not met. If message is empty, the default message is
//     used.
// Common exceptions:
//   - ex_write_error_document: thrown in cas the requirement is not met.
void require_access (db_access_t req, const string &message="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks if a certain access level or higher is present.
	 * Parameters:
	 *   - req: the access level required.
	 */
{
	if (!provides (session_access, req))
	{
		if (message.empty ())
			throw ex_write_error_document (
				"Unzureichender Datenbankzugriff"
				" (vorhanden: "+db_access_string (session_access)+";"
				" ben�tigt: "+db_access_string (req)+")"
				);
		else
			throw ex_write_error_document (message);

	}
}
/*}}}*/

void require_club_admin (const string &message="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks whether club admin permissions are present. The sk_admin and root
	 * users always has club admin permissions.
	 */
{
	switch (session_access)
	{
		case dba_access: case dba_none:
			throw ex_write_error_document (message_or_default (message, "Vereinsadministratorrechte ben�tigt"));
		case dba_root: return; break;
		case dba_sk_admin: return; break;
		case dba_sk_user:
			if (!session_user)
				throw ex_write_error_document ("session_user nicht gesetzt").is_program_error ();
			if (!session_user->perm_club_admin)
				throw ex_write_error_document (message_or_default (message, "Vereinsadministratorrechte ben�tigt"));
			return;
			break;
	}

	throw ex_write_error_document ("Unbehandelte Fall in require_club_admin").is_program_error ();
}
/*}}}*/

void require_read_flight_db (const string &message="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks whether permissions for reading the flight database are present.
	 * The sk_admin and root users always have these permissions.
	 */
{
	switch (session_access)
	{
		case dba_access: case dba_none:
			throw ex_write_error_document (message_or_default (message, "Unzureichende Zugriffsrechte"));
		case dba_root: return; break;
		case dba_sk_admin: return; break;
		case dba_sk_user:
			if (!session_user)
				throw ex_write_error_document ("session_user nicht gesetzt").is_program_error ();
			if (!session_user->perm_read_flight_db)
				throw ex_write_error_document (message_or_default (message, "Unzureichende Zugriffsrechte"));
			return;
			break;
	}

	throw ex_write_error_document ("Unbehandelte Fall in require_read_flight_db").is_program_error ();
}
/*}}}*/

void require_matching_club_admin (const string &club_name, const string &message="", const string &state="")/*{{{*/
	throw (ex_write_error_document, ex_go_to_state)
	/*
	 * Checks whether the user logged in is club admin for a given club.  The
	 * sk_admin and root users are always club admins for any club.
	 * Parameters:
	 *   - club_name: the name of the club that the user club must match.
	 *   - state: the state to go to instead of outputting an error, if not
	 *     empty.
	 * Exceptions:
	 *   - ex_go_to_state: thrown if the requirement is not met and state is
	 *     not empty.
	 */
{
	require_club_admin ();

	if (session_access==dba_sk_user)
	{
		if (session_user->club!=club_name)
		{
			string mess=message_or_default (message, "Falscher Verein");
			if (state.empty ())
				throw ex_write_error_document (mess);
			else
				throw ex_go_to_state (state).with_error_message (mess);
		}
	}
}
/*}}}*/

void require_sk_admin (const string &message="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks whether the user logged in has permission sk_admin.
	 */
{
	// For club admin, we need sk_user
	require_access (dba_sk_admin, message);
}
/*}}}*/

void require_not_self (const string &username, const string &message="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks whether the name of the user logged in does *not* match a given
	 * name.
	 * Paremters:
	 *   - username: the name that must not be matched.
	 */
{
	if (session_username==username)
		throw ex_write_error_document (message_or_default (message, "Diese Aktionen kann nicht mit sich selbst durchgef�hrt werden"));
}
/*}}}*/

void require_sk_user (const string &message="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks whether the user logged in has *exactly* the permission sk_user.
	 * This is used for operations which require information from a user, for
	 * example, outputting logbooks.
	 */
{
	// Requires that the session access is *exactly* dba_sk_user.
	if (session_access!=dba_sk_user)
		throw ex_write_error_document (message_or_default (message, "Datenbankzugriff muss dba_sk_user sein."));
}
/*}}}*/



user_class_t determine_user_class (const string &name)/*{{{*/
	/*
	 * Determines to which user class a given login name belongs.
	 * Parameters:
	 *   - name: the user name.
	 * Return value:
	 *   - The user class of the user.
	 */
{
	if (name.empty ())
		return uc_none;
	else if (name==opts.sk_admin_name)	// sk_admin_name might start with "sk_".
		return uc_mysql_user;
	else if (starts_with (name, "sk_"))
		return uc_sk_user;
	else if (name==opts.root_name)
		return uc_mysql_user;
	else if (name==opts.username)
		return uc_none;	// User startkladde cannot log in right now
	else
		return uc_sk_user;
}
/*}}}*/

bool authenticate (const string &username, const string &password, string &error_message)/*{{{*/
	/*
	 * Find out whether the user is allowed to log in.
	 * Parameters:
	 *   - the username and the password.
	 * Parameters set:
	 *   - error_message: a description of the error when false is returned.
	 * Return value: true if the user is allowed to log in.
	 * Variables updated: session_access
	 */
{
	// Now, lemme see. We will distinguish between the following cases:
	//   - login as a mysql user
	//     - root
	//     - sk_admin user (if defined in the config file)
	//     - startkladde user (if defined in the config file) (not right now)
	//     All other mysql user cannot log ing.
	//     The names of these accounts can be overridden in the config file or
	//     on the command line.
	//   - login as sk_user
	//     These are defined in the "user" table of the startkladde database.
	//     We need a MySQL account with fairly extensive permissions because
	//     these users may do things like, for example, write and modify master
	//     data.  These user don't exist as MySQL users because they should not
	//     be able to do anything they have write permissions for, for example,
	//     the club admins may only write master data for their own club and
	//     MySQL does not allow granting permission to individual rows.
	//     Additionally, some checking of the data (done by this program) may
	//     be necessary.
	//     So we use the startkladde_admin account for logging in in this case.
	//   - no login allowed
	// Which case applies is determined by the determine_user_class function.

	user_class_t user_class=determine_user_class (username);
	int ret;

#define LOGIN_ERROR(TEXT)	\
		{	\
			error_message=TEXT;	\
			return false;	\
		}

#define LOGIN_ERROR_IF(CONDITION, TEXT)	\
		if (CONDITION) LOGIN_ERROR (TEXT)

	switch (user_class)
	{
		case uc_none:
		{
			// Now this is easy. The user is not allowed to log in.
			LOGIN_ERROR ("Anmeldung f�r Benutzer "+username+" nicht erlaubt.");
			session_access=dba_none;
		} break;
		case uc_mysql_user:
		{
			// Log in as this very user.
			try
			{
				db.set_user_data (username, password);
				db.connect ();
				db.use_db ();

				if (username==opts.root_name)
					session_access=dba_root;
				else if (username==opts.sk_admin_name)
					session_access=dba_sk_admin;
				else
					LOGIN_ERROR ("Unbehandelter MySQL-Benutzer (Programmfehler)");

				return true;
			}
			catch (sk_exception &e)
			{
				LOGIN_ERROR (e.description ());
			}

		} break;
		case uc_sk_user:
		{
			// This is the interesting case: log in as sk user. This means that
			// there is not corresponding MySQL user, so we log in as sk_admin
			// to authenticate the user using sk_db sk_user functions.

			// Check whether we have sk_admin access data at all.
			LOGIN_ERROR_IF (opts.sk_admin_name.empty (), "Name f�r das Administratorkonta (sk_admin) nicht konfiguriert");
			LOGIN_ERROR_IF (opts.sk_admin_password.empty (), "Passwort f�r '"+opts.sk_admin_name+"' nicht konfiguriert");

			// Try to log in to the server as user sk_admin.
			if (db.connected ()) db.disconnect ();
			try
			{
				db.set_user_data (opts.sk_admin_name, opts.sk_admin_password);
				db.connect ();
				db.use_db ();
			}
			catch (sk_exception &e)
			{
				LOGIN_ERROR_IF (true, "sk_admin: "+e.description ());
			}

			// Now that we are logged in, we can proceed to authenticating the user.
			LOGIN_ERROR_IF (!db.sk_user_exists (username), "Der Benutzer \""+username+"\" existiert nicht");
			LOGIN_ERROR_IF (!db.sk_user_authenticate (username, password), "Falsches Passwort");

			// If we are here, authentication succeeded.
			// TODO: actually, this is also code duplication with setup_variables
			session_access=dba_sk_user;

			// Now we can also set session_user
			// Use a temporary pointer to keep session_user const.
			if (session_user) delete session_user; session_user=NULL;
			sk_user *_session_user=new sk_user;
			ret=db.sk_user_get (*_session_user, username);
			LOGIN_ERROR_IF (ret!=db_ok, db.db_error_description (ret, true));
			session_user=_session_user;

			return true;
		} break;
	}
	// You are not supposed to be here.
	LOGIN_ERROR ("Unbehandelte Benutzerklasse in authenticate ()");

#undef LOGIN_ERROR
#undef LOGIN_ERROR_IF
}
/*}}}*/

argument_list make_persistent_parameter_list (const string &state_label)/*{{{*/
	/*
	 * Persistent parameters are those which need to be carried on the the next
	 * state. This functions make a list of persistent parameters.
	 * Parameters:
	 *   - state_label: the state to go to, this will also be added to the list.
	 *     It may also be used to determine the set of persistent parameters.
	 * Return value:
	 *   - The list.
	 */
{
	argument_list args;

	// Add the state if not empty
	if (!state_label.empty ())
		args.set_value (arg_cgi_new_state, state_label);

	// Add the session ID to the parameter list (if present)
	if (session.is_ok ())
		args.set_value (arg_cgi_session_id, session.get_id ());

	// Add the debug flag to the paramter list (if present)
	// Don't use debug_enabled here because it might have been enabled
	// by other means than CGI.
	if (CGI_HAS (debug)) args.set_value (arg_cgi_debug, CGI_READ (debug));
	if (CGI_HAS (no_redirects)) args.set_value (arg_cgi_no_redirects, CGI_READ (no_redirects));

	return args;
}
/*}}}*/

string back_form_hidden (string new_state_label)/*{{{*/
	/*
	 * Returns a string containing HTML hidden inputs for going to a new state,
	 * includin the persistent parameters.
	 * Parameters:
	 *   - new_state_label: the state to go to.
	 * Return value:
	 *   The string with the hidden labels.
	 */
{
	// Make the parameter list
	argument_list parameters=make_persistent_parameter_list (new_state_label);

	// TODO move tag writing to html class
	string r;
	argument_list::const_iterator end=parameters.get_list ().end ();
	for (argument_list::const_iterator it=parameters.get_list ().begin (); it!=end; ++it)
		r+="<div><input type=\"hidden\" name=\""+html_escape ((*it).get_name ())+"\" value=\""+html_escape ((*it).get_value ())+"\"></div>\n";

	return r;
}
/*}}}*/

string back_link_url (const string &new_state_label, const argument_list &parameters, const string &anchor="", bool absolute=false)/*{{{*/
	/*
	 * Makes the URL for linking to another state.
	 * Parameters:
	 *   - new_state_label: the state to go to.
	 *   - parameters: additional parameters to add.
	 *   - anchor: the anchor in the page to jump to (...#name in URL)
	 *   - absolute: whether to use the absolute URL. Required (?) for
	 *     redirects.
	 */
{
	string url;
	if (absolute)
		url=absolute_url;
	else
		url=relative_url;

	// Make the parameter list
	argument_list params=make_persistent_parameter_list (new_state_label);
	params.add (parameters);

	// Add the parameters
	if (!params.empty ()) url+="?"+params.make_cgi_parameters ();

	// Add the anchor
	if (!anchor.empty ()) url+="#"+anchor;

	// Return the complete URL
	return url;
}
/*}}}*/

string back_link_url (const string &new_state_label, const string &anchor="", bool absolute=false)/*{{{*/
	/*
	 * Like back_link_url above, but without the parameters list. No additional
	 * parameters are added.
	 */
{
	argument_list parameters;
	return back_link_url (new_state_label, parameters, anchor, absolute);
}
/*}}}*/

what_next make_redirect (const string &new_state_label, const argument_list &args, const string &anchor="")/*{{{*/
	/*
	 * Makes a redirect action to another state.
	 * Parameters:
	 *   - new_state_label: the state to go to.
	 *   - args: additional arguments to add.
	 *   - anchor: the anchor to jump to in the document.
	 * Return value:
	 *   the redirect action.
	 */
{
	// For redirects, it seems we need to use an absolute URL.
	string url=back_link_url (new_state_label, args, anchor, true);
	return what_next::do_redirect (url);
}
/*}}}*/

what_next make_redirect (const string &new_state_label, const string &anchor="")/*{{{*/
	/*
	 * Like make_redirect above, but without the additional argument list. No
	 * additional arguments will be added.
	 */
{
	argument_list args;
	return make_redirect (new_state_label, args, anchor);
}
/*}}}*/

string logout_link ()/*{{{*/
	throw ()
	/*
	 * Creates an URL for logging out of the web interface.
	 * Return value:
	 *   - the URL.
	 */
{
	return document.text_link (back_link_url (web_logout), "Abmelden");
}
/*}}}*/

void write_document_footer ()/*{{{*/
	/*
	 * Writes the document footer. The document footer may consist of, possibly
	 * among others:
	 *   - information about the user logged in
	 *   - a logout link
	 *   - a link to the main menu
	 *   - version information
	 *   - debug information, if debug_enabled is set
	 *   - information about the environment
	 */
{
	document.write_hr ();

	document.start_paragraph ();
	if (session.is_ok ())
	{
		document
			.write_text ("Angemeldet als "+session.args.get_value (arg_session_login_name)+" (")
			.write (logout_link ())
			.write_text (")")
			;
	}
	else
	{
		document
			.write_text ("Nicht angemeldet (")
			.write_text_link (back_link_url (web_login), "Anmelden")
			.write_text (")")
			;
	}

	if (!current_state || current_state->get_label ()!=web_main_menu)
		document
			.write_br ()
			.write_text_link (back_link_url (web_main_menu), "Zur�ck zum Hauptmen�")
			;

	document.end_paragraph ();

	document.write_paragraph ("<small>sk_web "+version_info ()+"</small>");

	if (debug_enabled)
	{
		document.write_hr ();

		// Write the debug output
		string debug_string=debug_stream.str ();
		if (!debug_string.empty ())
			document.write ("<p><pre>\n").write (debug_string, false).write ("</pre></p>\n");

		// Write the CGI argument list
		document.start_paragraph ();
		const list<argument> &cgi_args_list=cgi_args.get_list ();
		argument_list::const_iterator end=cgi_args_list.end ();
		for (argument_list::const_iterator it=cgi_args_list.begin (); it!=end; ++it)
		{
			string name=(*it).get_name ();
			string value;
			if (name.find ("pass")==string::npos)
				value="\""+((*it).get_value ())+"\"";
			else if ((*it).get_value ().empty ())
				value="";
			else
				value="***";

			document.write (html_escape (name+"="+value)+"<br>\n");
		}
		document.end_paragraph ();

		// Write connection information
		document
			.start_paragraph ()
			.write ("Server: "+opts.server_display_name).write_br ()
			.write ("Port: "+num_to_string (opts.port)).write_br ()
			.write ("Datenbank: "+opts.database).write_br ()
			.end_paragraph ();

		// Nope, this ain't gonna work. Need to output this to the document
		// (and html_escape).
//		document.write ("<p><small><pre>\n");
//		system ("export");
//		document.write ("</pre></small></p>\n");

		// This is unsafe because it displays the query string in clear text,
		// including passwords.
		//document.write_paragraph (query_string);
	}
}
/*}}}*/

string make_anchor_name (db_id id)/*{{{*/
	/*
	 * Makes an anchor name for object lists which have db_ids.
	 * Parameters:
	 *   - id: the object id.
	 * Return value:
	 *   The name of the anchor.
	 */
{
	return "id_"+num_to_string (id);
}
/*}}}*/

// CHECK_DB_ERROR_ERROR/*{{{*/
	/*
	 * Checks the return value of a database call and returns an error action
	 * if the return value sigifies an error.
	 * Variables used:
	 *   - ret: the return value of a database call.
	 */
// TODO: replace this by DO_DB_ACTION (action (args)) which throws an
// exception on error. Better yet a function.
#define CHECK_DB_ERROR_ERROR	\
	do { if (ret<0) return what_next::output_error (db.db_error_description (ret, true)); } while (false)
/*}}}*/

// CHECK_DB_ERROR_STATE(STATE)/*{{{*/
	/*
	 * Checks the return value of a database call and returns a state change
	 * action if the return value sigifies an error.
	 * Parameters:
	 *   - STATE: the state to go to
	 * Variables used:
	 *   - ret: the return value of a database call.
	 */
#define CHECK_DB_ERROR_STATE(STATE)	\
	do { if (ret<0) return what_next::go_to_state (STATE, db.db_error_description (ret), true); } while (false)
/*}}}*/

// CHECK_AN_ID/*{{{*/
	/*
	 * Checks an ID and outputs an error state if it is invalid. The error
	 * message contains the value given for the ID as read from the CGI
	 * parameters.
	 * Paremters:
	 *   - ID: the ID to check
	 *   - CGI_ARG: the CGI argument to read the given value from. This is
	 *     passed to CGI_READ, so arg_cgi_ is prepended to the name.
	 */
#define CHECK_AN_ID(ID, CGI_ARG)	\
	do { if (id_invalid (ID)) return what_next::output_error ("Ung�ltige ID \""+CGI_READ (CGI_ARG)+"\""); } while (false)
/*}}}*/

// CHECK_ID/*{{{*/
	/*
	 * Checks an ID which is contained in the variable called id and was read
	 * from the CGI argument arg_cgi_id.
	 */
#define CHECK_ID CHECK_AN_ID (id, id);
/*}}}*/

what_next csv_to_persons (const string &csv, QPtrList<sk_person> &persons, const string &club)/*{{{*/
	/*
	 * Converts a string containing a CSV file to a list of persons.
	 * This uses session_user, so only call this when the access level is
	 * (exactly) sk_user. An error is output if this is not the case.
	 * Parameters:
	 *   - csv: the CSV file contents
	 *   - persons: the person list to write
	 *   - club: the club to write to the persons
	 */
{
	require_sk_user ();

	istringstream csv_stream (csv);

	table_row header_row=table_row::from_csv (csv_stream);

	// Check the CSV fields
	// Make a list of fields that must be present.
	list<string> missing_fields;
	missing_fields.push_back (csv_field_person_last_name);
	missing_fields.push_back (csv_field_person_first_name);
	missing_fields.push_back (csv_field_person_club_id);

	// Remove headers which are actually present from the list.
	table_row::const_iterator header_row_end=header_row.end ();
	// TODO case insensitivity
	for (table_row::const_iterator it=header_row.begin (); it!=header_row_end; ++it)
		missing_fields.remove (*it);

	// If the list still contains elemnts, this is an error.
	if (!missing_fields.empty ())
	{
		// Output a message listing the missing columns
		// TODO symbolic constant for "error"
		document.write_paragraph (html_escape ("Fehler: die folgenden Felder fehlen in der CSV-Datei:"), "error");
		document.write_text_list (missing_fields, false);

		// Write a link back to the file selection page
		argument_list additional_args;
		additional_args.set_value (arg_cgi_data_type, CGI_READ (data_type));
		document
			.start_paragraph ()
			.write_text_link (back_link_url (web_master_data_import, additional_args), "Zur�ck")
			.end_paragraph ()
			;
		return what_next::output_document ();
	}

	// OK, all required columns are present.

	// Parse the CSV file to a table
	table csv_table=table::from_csv (csv_stream);

	// TODO better field handling
	// Find out where which field is
	int index_last_name=-1;
	int index_first_name=-1;
	int index_comments=-1;
	int index_club_id=-1;
	int index_club_id_old=-1;

	int num_columns=header_row.size ();
	for (int i=0; i<num_columns; ++i)
	{
		// If the i-th field of the header row has a certain title, set a
		// certain variable to i.
#define TEST(NAME) do { if (header_row.at (i)==csv_field_person_ ## NAME) index_ ## NAME=i; } while (false)
		TEST (last_name);
		TEST (first_name);
		TEST (comments);
		TEST (club_id);
		TEST (club_id_old);
#undef TEST
	}

	// Make a list of persons from the table
	// Legacy: persons are listed in QPtrList instead of a std::list
	// because the database still uses this type and so do the person
	// handling functions in sk_web.
	table::const_iterator table_end=csv_table.end ();
	for (table::const_iterator it=csv_table.begin (); it!=table_end; ++it)
	{
		sk_person *p=new sk_person;
#define WRITE(NAME,TARGET) do { if (index_ ## NAME>=0 && ((int)(*it).size ())>index_ ## NAME) p->TARGET=(*it).at (index_ ## NAME); } while (false)
		WRITE (last_name, nachname);
		WRITE (first_name, vorname);
		WRITE (comments, bemerkungen);
		WRITE (club_id, club_id);
		WRITE (club_id_old, club_id_old);
#undef WRITE

		p->club=club;

		persons.append (p);
	}

	return what_next::go_on ();
}
/*}}}*/

void make_unique_club_list (list<string> &club_list, QPtrList<sk_flugzeug> plane_list)/*{{{*/
	/*
	 * For each plane, adds the club to the club list, if not already present.
	 * Parameters:
	 *   - club_list: the list of clubs.
	 *   - plane_list: the list of planes.
	 */
{
	for (QPtrListIterator <sk_flugzeug> plane (plane_list); *plane; ++plane)
	{
		// Don't use .grep here because we need to simplify_club_name
		bool already_has=false;
		QString plane_club=std2q ((*plane)->club);
		if (eintrag_ist_leer (plane_club)) plane_club="-";
		for (list<string>::iterator club=club_list.begin (); club!=club_list.end (); ++club)
		{
			if (simplify_club_name (*club)==simplify_club_name (q2std (plane_club)))
			{
				already_has=true;
				break;
			}
		}
		if (!already_has) club_list.push_back (q2std (plane_club));
	}
}
/*}}}*/





bool user_name_valid (const string &name)/*{{{*/
	/*
	 * Checks whether a user name is valid (e. g., does not contain any invalid
	 * characters).
	 * Paramters:
	 *   - name: the user name to check.
	 * Return value:
	 *   true if the user name is valid, false else.
	 */
{
	// As of 2005-01-11, the program seems to be able to handle '\'' and '"' in
	// user names, but not blanks. Users with names containing blanks cannot be
	// edited. There is a "user not found" message, although the query executed
	// seems to be OK.
	// Nevertheless, we forbid characters like '\'' and '"' as well.
	if (name.find_first_not_of ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_.-")==string::npos)
		return true;
	else
		return false;
}
/*}}}*/

what_next redirect_to_result (string result_text, bool result_error=false, string next_state_label="")/*{{{*/
	/*
	 * Makes a redirect to the result page. A message is passed to the result
	 * state via the session.
	 * Paramters:
	 *   - result_text: the text to display on the result page.
	 *   - result_error: whether the result is an error.
	 *   - next_state_label: if not empty, a link to this state will be
	 *     displayed on the result page.
	 * Return value:
	 *   The redirect action.
	 */
{
	SESSION_WRITE (result_text, result_text);
	if (!next_state_label.empty ()) SESSION_WRITE (result_state, next_state_label);
	if (result_error) SESSION_WRITE (result_error, "");
	return make_redirect (web_result);
}
/*}}}*/

// DO_SUB_ACTION(ACTION)/*{{{*/
	/*
	 * Calls a function that returns a what_next and returns this what_next
	 * unless it is wn_go_on.
	 */
// TODO replace this by action throwing an exception, then remove
// wn_go_on.
#define DO_SUB_ACTION(ACTION)	\
do	\
{	\
	what_next n=ACTION;	\
	if (n.get_next ()!=wn_go_on) return n;	\
} while (false)
/*}}}*/

html_table_row make_table_header (list<object_field> fields)/*{{{*/
	/*
	 * Makes a table header suitable for tables listing objects.
	 * Paramters:
	 *   - fields: the object fields containing the data.
	 * Return value:
	 *   The table row.
	 */
{
	html_table_row row;

	list<object_field>::const_iterator fields_end=fields.end ();
	// For each entry in the fields list, write a header cell
	for (list<object_field>::const_iterator field=fields.begin (); field!=fields_end; ++field)
		// Only write if the field should be visible
		if ((*field).get_list_display ())
			row.push_back (html_table_cell::text ((*field).get_caption (), true));

	return row;
}
/*}}}*/

html_table_row make_table_data_row (list<object_field> fields)/*{{{*/
	/*
	 * Makes a table data row suitable for tables listing objects.
	 * Paramters:
	 *   - fields: the object fields containing the data.
	 * Return value:
	 *   The table row.
	 */
{
	// TODO move to HTML table?
	// use table (table_row_from_fields)?
	html_table_row row;

	list<object_field>::const_iterator fields_end=fields.end ();
	// For each field, write the data cell.
	for (list<object_field>::const_iterator field=fields.begin (); field!=fields_end; ++field)
		if ((*field).get_list_display ())
		{
			string text=html_escape ((*field).make_display_text ());
			if ((*field).get_no_break ()) text="<nobr>"+text+"</nobr>";
			row.push_back (html_table_cell (text));
		}

	return row;
}
/*}}}*/

table_row table_row_from_fields (list<object_field> fields, bool header=false)/*{{{*/
	/*
	 * Converts a list of object fields to a table row. The table row contains
	 * either the string representations from the object fields or the
	 * captions.
	 * Paramters:
	 *   - fields: the list of fields containing data for an object.
	 *   - header: whether to use headers or data.
	 * Return value:
	 *   The table row.
	 */
{
	// TODO move to table? move to object_field?

	table_row row;

	// For each field, write the data cell.
	list<object_field>::const_iterator fields_end=fields.end ();
	for (list<object_field>::const_iterator field=fields.begin (); field!=fields_end; ++field)
	{
		if (header)
		{
			row.push_back (table_cell ((*field).get_caption ()));
		}
		else
		{
			if ((*field).get_list_display ())
				row.push_back (table_cell ((*field).make_display_text ()));
		}
	}

	return row;
}
/*}}}*/

html_table_cell make_link_cell (const string &next_state, const string &caption, const string &argname, const string &argvalue)/*{{{*/
	/*
	 * Makes a cess containing a link to another state.
	 * Paramters:
	 *   - next_state: the state to link to.
	 *   - caption: the text of the link.
	 *   - argname: the name of an additional argument to pass.
	 *   - argvalue: value of the argument argname.
	 */
{
	argument_list args;
	args.set_value (argname, argvalue);
	return html_table_cell (document.text_link (back_link_url (next_state, args), caption));
}
/*}}}*/

void write_delete_links (const string &delete_state, const string &ident_arg, const string &ident_val, const string &back_state, const string &anchor)/*{{{*/
	/*
	 * Write links to the document asking the user for deletion confirmation,
	 * that is, a
	 * "really delete" link and a "back" link.
	 * Parameters:
	 *   - delete_state: the state to go to for deleting.
	 *   - ident_arg: the argument used for identifying the object to be deleted.
	 *   - ident_val: the value for ident_arg.
	 *   - back_state: the state to go back to if not deleting.
	 *   - anchor: the anchor to jump to on the back page, if not empty.
	 */
{
	argument_list delete_args;
	delete_args.set_value (ident_arg, ident_val);
	document
		.write_text_link (back_link_url (delete_state, delete_args), "Wirklich l�schen")
		.write (" ")
		.write_text_link (back_link_url (back_state, anchor), "Zur�ck");
}
/*}}}*/

void check_username (const string &username)/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Checks the user name and outputs an error if it is invalid.
	 * Paramters:
	 *   - username: the name to check.
	 * Exceptions:
	 *   - ex_write_error_document: thrown when the user name is not valid.
	 */
{
	if (!user_name_valid (username)) throw ex_write_error_document ("Benutzername \""+username+"\" enth�lt ung�ltige Zeichen");
}
/*}}}*/


// Output handling
void setup_latex_headings (latex_document &ldoc, const string &caption, const string &date_text)/*{{{*/
{
	ldoc.font_size=8;
	ldoc.head_ro=date_text;
	ldoc.head_co=latex_escape (caption);
	ldoc.foot_ro="Seite \\thepage{} von \\pageref{LastPage}";
	ldoc.foot_lo="sk\\textunderscore{}web "+latex_escape (version_info ());
	ldoc.foot_co="";
}
/*}}}*/

// TODO use the add_* functions everywhere, also where writing forms based on
// object_fields
void add_date_inputs (html_table &table, bool include_range)/*{{{*/
{
	// TODO add possibility to include any combination, use hidden
	// instead of radio button if there is only one option,
	// make configurable which radio button is preselected.

	// Determine the default values for dates
	sk_date current_date=sk_date::current ();
	string now_year=num_to_string (current_date.get_year ());
	string now_month=num_to_string (current_date.get_month ());
	string now_day=num_to_string (current_date.get_day ());

	html_table_row row;

	row.clear ();
	row.push_back (html_table_cell::text ("Datum:"));
	row.push_back (html_table_cell (document.make_input_radio (arg_cgi_date_spec, arg_cgi_date_spec_today, "Heute", true)+"<br>", false));
	table.push_back (row);

	string single_date_text=include_range?"Ein Tag:":"Anderes Datum";

	row.clear ();
	row.push_back (html_table_cell::empty ());
	row.push_back (html_table_cell (document.make_input_radio (arg_cgi_date_spec, arg_cgi_date_spec_single, single_date_text, false)+"<br>"));
	if (include_range) row.push_back (html_table_cell::text ("Datum:"));
	row.push_back (html_table_cell (html_escape ("Jahr: ")+document.make_input_text (arg_cgi_date_single_year, now_year, 4)));
	row.push_back (html_table_cell (html_escape ("Monat: ")+document.make_input_text (arg_cgi_date_single_month, now_month, 2)));
	row.push_back (html_table_cell (html_escape ("Tag: ")+document.make_input_text (arg_cgi_date_single_day, now_day, 2)));
	table.push_back (row);

	if (include_range)
	{
		row.clear ();
		row.push_back (html_table_cell::empty ());
		row.push_back (html_table_cell (document.make_input_radio (arg_cgi_date_spec, arg_cgi_date_spec_range, "Bereich:", false)+"<br>"));
		row.push_back (html_table_cell::text ("Anfang:"));
		row.push_back (html_table_cell (html_escape ("Jahr: ")+document.make_input_text (arg_cgi_date_start_year, now_year, 4)));
		row.push_back (html_table_cell (html_escape ("Monat: ")+document.make_input_text (arg_cgi_date_start_month, "1", 2)));
		row.push_back (html_table_cell (html_escape ("Tag: ")+document.make_input_text (arg_cgi_date_start_day, "1", 2)));
		table.push_back (row);

		row.clear ();
		row.push_back (html_table_cell::empty ());
		row.push_back (html_table_cell::empty ());
		row.push_back (html_table_cell::text ("Ende:"));
		row.push_back (html_table_cell (html_escape ("Jahr: ")+document.make_input_text (arg_cgi_date_end_year, now_year, 4)));
		row.push_back (html_table_cell (html_escape ("Monat: ")+document.make_input_text (arg_cgi_date_end_month, now_month, 2)));
		row.push_back (html_table_cell (html_escape ("Tag: ")+document.make_input_text (arg_cgi_date_end_day, now_day, 2)));
		table.push_back (row);
	}
}
/*}}}*/

void add_submit_input (html_table &table, const string &text="Abrufen")/*{{{*/
{
	html_table_row row;
	row.push_back (html_table_cell (document.make_submit (text), false, 0));
	table.push_back (row);
}
/*}}}*/

void add_bool_input (html_table &table, const string &caption, const string &label, bool default_value=false)/*{{{*/
{
	html_table_row row;

	row.push_back (html_table_cell::text (caption+string (":")));
	row.push_back (html_table_cell (document.make_input_select_bool (label, default_value)));
	table.push_back (row);
}
/*}}}*/

void add_select_input (html_table &table, const string &caption, const string &label, const argument_list &options, const string &def="")/*{{{*/
{
	html_table_row row;
	row.push_back (html_table_cell::text (caption));
	row.push_back (html_table_cell (document.make_input_select (label, options, def), false, 0));
	table.push_back (row);
}
/*}}}*/

// Treat person
/*
 * These functions do a certain action with a person. They are deprecated for
 * anything else than a user in favor of the object_field mechanism, or rather
 * an object_field successor. Even for persons, this should be replaced onece
 * an object_fields successor is implemented.
 * Note (I repeat): these functions are obsolete.
 * Common parameters:
 *   - treatment: the action to perform.
 */
void treat_preamble (treatment_t treatment, string anchor_name)/*{{{*/
	/*
	 * Writes the introduction for a treatment of a single object, if any (for
	 * example, table row beginning tags).
	 * Parameters:
	 *   - anchor_name: the name of the anchor to place for tables. If empty,
	 *     no anchor is placed.
	 * See also:
	 *   -treat_appendix
	 */
{
	string anchor_string;
	if (!anchor_name.empty ()) anchor_string=document.anchor (anchor_name);

	switch (treatment)
	{
		case tm_write_table_prefix: document.write ("<p><table>\n"); break;
		case tm_write_table_suffix: document.write ("</table></p>\n"); break;
		case tm_write_table_header: document.write ("<tr>\n"); break;
		case tm_write_table_data: document.write ("<tr>"+anchor_string+"\n"); break;
		case tm_write_display: document.write ("<table class=\"noborder\">"); break;
		case tm_write_edit: case tm_write_create:
			document.write (
				"<form action=\""+relative_url+"\" method=\"POST\">\n"
				"<table>\n"
				);
		   break;
		case tm_edit: break;
		case tm_create: break;
	}
}
/*}}}*/

void treat_field (treatment_t treatment, const string &caption, const string &cginame, const string &string_value, string *string_member, bool bool_value, bool *bool_member, const string &link_to)/*{{{*/
	/*
	 * Handle a treatment for a single field.
	 * This function takes a lot of parameters not all of which are used for
	 * all kinds of treatment.
	 * bool_value is used if bool_member is not NULL.
	 * XXX
	 */
{
	string value_text=bool_member?bool_to_string (bool_value):string_value;

	switch (treatment)
	{
		case tm_write_table_prefix:
			break;
		case tm_write_table_suffix:
			break;
		case tm_write_table_header:
			document.write ("  <th>"+caption+"</th>\n");
			break;
		case tm_write_table_data:
			if (link_to.empty ())
				document.write ("  <td>"+html_escape (value_text)+"</td>\n");
			else
				document.write ("  <td>"+document.text_link (link_to, html_escape (value_text))+"</td>\n");
			break;
		case tm_write_display:
			document.write (
					"  <tr>\n"
					"    <td>"+html_escape (caption)+":</td>\n"
					"    <td>"+html_escape (value_text)+"</td>\n"
					"  </tr>\n"
					);
			break;
		case tm_write_edit:
			if (string_member)
				document.write (
						"  <tr>\n"
						"    <td>"+html_escape (caption)+":</td>\n"
						"    <td>"+document.make_input_text (cginame, string_value)+"</td>\n"
						"  </tr>\n"
						);
			else if (bool_member)
				document.write (
						"  <tr>\n"
						"    <td>"+html_escape (caption)+":</td>\n"
						"    <td>"+document.make_input_select_bool (cginame, bool_value)+"</td>\n"
						"  </tr>\n"
						);
			else
				treat_field (tm_write_display, caption, cginame, string_value, string_member, bool_value, bool_member, link_to);
			break;
		case tm_write_create:
			if (string_member)
				document.write (
						"  <tr>\n"
						"    <td>"+html_escape (caption)+":</td>\n"
						"    <td>"+document.make_input_text (cginame, string_value)+"</td>\n"
						"  </tr>\n"
						);
			else if (bool_member)
				document.write (
						"  <tr>\n"
						"    <td>"+html_escape (caption)+":</td>\n"
						"    <td>"+document.make_input_select_bool (cginame, bool_value)+"</td>\n"
						"  </tr>\n"
						);
			break;
		case tm_edit: case tm_create:
			// Even if the strings are empty, as long as they are present, they are set.
			if (string_member && CGI_HAS_F (cginame)) (*string_member)=CGI_READ_F (cginame);
			if (bool_member && CGI_HAS_F (cginame)) (*bool_member)=string_to_bool (CGI_READ_F (cginame));
			break;
	}
}
/*}}}*/

void treat_additional_fields (treatment_t treatment, const list<argument> &additional_fields)/*{{{*/
	/*
	 * Writes additional fields, for example, submit buttons or additional
	 * columns.
	 * Parameters:
	 *   - additional_fields: for tables, contains additional links to be
	 *     written. The name will be used as link text and the value as link
	 *     target.
	 */
{
	string link;
	switch (treatment)
	{
		case tm_write_table_prefix: break;
		case tm_write_table_suffix: break;
		case tm_write_table_header: break;
		case tm_write_table_data:
		{
			argument_list::const_iterator end=additional_fields.end ();
			for (argument_list::const_iterator it=additional_fields.begin (); it!=end; ++it)
			{
				document
					.start_tag ("td")
					.write_text_link ((*it).get_value (), (*it).get_name ())
					.end_tag ("td");
			}
		} break;
		case tm_write_display: break;
		case tm_write_edit:
			document.write (
				"<tr><td colspan=2>\n"
				"  <input type=\"submit\" value=\""+string ("�ndern")+"\">\n"
				"</td></tr>\n"
				);
			break;
		case tm_write_create:
			document.write (
				"<tr><td colspan=2>\n"
				"  <input type=\"submit\" value=\""+string ("Anlegen")+"\">\n"
				"</td></tr>\n"
				);
			break;
		case tm_edit: break;
		case tm_create: break;
	}
}
/*}}}*/

void treat_appendix (treatment_t treatment, const string &edit_next_state, const string &create_next_state, const list<argument> &additional_args)/*{{{*/
	/*
	 * Writes the appendix for a treatment of a single object, if any. This is
	 * the counterpart treat_preamble.
	 * Not all of the parameters are used for all treatments.
	 * Parameters:
	 *   - edit_next_state: the state to go to for editing.
	 *   - create_next_state: the state to go to for creating.
	 *   - additional_args: addigional CGI arguments to pass to the next state.
	 * See also:
	 *   -treat_preamble
	 */
{

	// Appendix
	switch (treatment)
	{
		case tm_write_table_prefix: break;
		case tm_write_table_suffix: break;
		case tm_write_table_header: document.write ("</tr>"); break;
		case tm_write_table_data: document.write ("</tr>"); break;
		case tm_write_display: document.write ("</table>"); break;
		case tm_write_edit:
			document
				.write ("</table>\n")
				.write (back_form_hidden (edit_next_state))	// TODO make back_form_hidden return a list for write_hidden_fields
				.write_hidden_fields (additional_args)
				.write ("</form>\n")
				;
			break;
		case tm_write_create:
			document
				.write ("</table>\n")
				.write (back_form_hidden (create_next_state))
				.write_hidden_fields (additional_args)
				.write ("</form>\n")
				;
			break;
		case tm_edit: break;
		case tm_create: break;
	}
}
/*}}}*/

//treat_person (person *, treatment, new_state, ident_arg, args, for_import, for_select, additional_text
what_next treat_person (sk_person *_p, treatment_t treatment, const string &new_state=web_display_person, const string &ident_arg=arg_cgi_id, const argument_list *args=NULL, bool for_import=false, bool for_select=false, const string &additional_text="")/*{{{*/
	/*
	 * Handle a treatment for a person.
	 * This function basically calls treat_field on all of the fields of
	 * a person.
	 * new_state and ident_arg are used for person_select
	 * XXX
	 * TODO: Problem her is that for some tratments, _p is written and for some
	 * it is not, this prevents const correctness.
	 */
{
	// Check
	if (!_p && object_required (treatment))
		return what_next::output_error ("Fehlende Person in treat_person (Programmfehler)");

	sk_person dummy;
	sk_person *p=_p;
	if (!p) p=&dummy;

	// Preamble
	treat_preamble (treatment, make_anchor_name (p->id));

	// Data
	// TODO not manually
	argument_list link_to_arguments;
	if (args) link_to_arguments.add (*args);
	link_to_arguments.set_value (ident_arg, num_to_string (p->id));

	string action_text="-";
	if (_p)
	{
		if (id_invalid (_p->id))
			action_text="(Neu anlegen)";
		else
			action_text=num_to_string (_p->id);
	}

	string none="---";
	// MURX
	if (treatment==tm_write_edit || treatment==tm_write_create) none="";

	string club_value;
	if (_p)
		club_value=p->club;
	else if (session_access==dba_sk_user)
		club_value=session_user->club;
	else
		club_value="";

	// _p: The parameter given, may not be given
	// p: The person buffer, is always present
	//                            treatment  caption           cginame             string_value                                 string_member      bool_value     bool_member               link_to)
	if (!for_import) treat_field (treatment, "ID",             "",                 _p?num_to_string (p->id):"(Keine)",          NULL,              false,         NULL,                     back_link_url (new_state, link_to_arguments));
	                 treat_field (treatment, "Nachname",       arg_cgi_last_name,  _p?p->nachname          :none,               &(p->nachname),    false,         NULL,                     "");
	                 treat_field (treatment, "Vorname",        arg_cgi_first_name, _p?p->vorname           :none,               &(p->vorname),     false,         NULL,                     "");
	                 treat_field (treatment, "Verein",         arg_cgi_club,       club_value,                                  &(p->club),        false,         NULL,                     "");
	                 treat_field (treatment, "Vereins-ID",     arg_cgi_club_id,    _p?p->club_id           :none,               &(p->club_id),     false,         NULL,                     "");
	if (for_import)  treat_field (treatment, "Vereins-ID alt", "",                 _p?p->club_id_old       :none,               &(p->club_id_old), false,         NULL,                     "");
	                 treat_field (treatment, "Bemerkungen",    arg_cgi_comments,   _p?p->bemerkungen       :none,               &(p->bemerkungen), false,         NULL,                     "");
	if (!for_import) treat_field (treatment, "Editierbar",     arg_cgi_editable,   _p?""                   :none,               NULL,              p->editierbar, &(p->editierbar),         "");
	if (for_import)  treat_field (treatment, "Person",         "",                 action_text                   ,              NULL,              false,         NULL,                     "");
	if (!additional_text.empty ())
	                 treat_field (treatment, additional_text,  "",                 additional_text,                             NULL,              false,         NULL,                     "");

	// Additional fields
	if (!for_import && !for_select)
	{
		argument_list fields;
		if (args) fields.add (*args);
		// TODO don't specify manually
		fields.set_value ("Editieren", back_link_url (web_edit_person)+"&"+arg_cgi_id+"="+num_to_string (p->id));
		fields.set_value ("L�schen", back_link_url (web_delete_person)+"&"+arg_cgi_id+"="+num_to_string (p->id));
		fields.set_value ("�berschreiben", back_link_url (web_select_merge_person)+"&"+arg_cgi_id+"="+num_to_string (p->id));
		treat_additional_fields (treatment, fields.get_list ());
	}

	// Appendix
	argument_list appendix_args;
	appendix_args.set_value (ident_arg, num_to_string (p->id));
	treat_appendix (treatment, web_do_edit_person, web_do_create_person, appendix_args.get_list ());

	return what_next::go_on ();
}
/*}}}*/

what_next write_person_list (QPtrList<sk_person> &persons, bool include_none, const string &target_state, const string &result_field, const argument_list *args=NULL, bool for_import=false, bool for_select=false)/*{{{*/
	/*
	 * Writes a person list (table).
	 * this function is obsolete as it uses the treat_* mechanism.
	 * Paramters:
	 *   - persons: the person list to write
	 *   - include_none: whether to write a row with a "none" entry.
	 *   - target_state: the state to go to when selecting a person.
	 *   - result_field: the name of the argument to pass the ID in.
	 *   - args: additional arguments to pass.
	 *   - for_import: whether to write the list for importing (MURX)
	 *   - for_select: whether to write the list for selecting (MURX)
	 * Return value:
	 *   - wn_go_on on success.
	 *   - something else else.
	 */
{
	DO_SUB_ACTION (treat_person (NULL, tm_write_table_prefix));
	DO_SUB_ACTION (treat_person (NULL, tm_write_table_header, "", "", NULL, for_import));
	if (include_none) DO_SUB_ACTION (treat_person (NULL, tm_write_table_data, target_state, result_field, args, for_import, for_select));	// Write "none"
	for (QPtrListIterator<sk_person> person (persons); person; ++person)
		DO_SUB_ACTION (treat_person (*person, tm_write_table_data, target_state, result_field, args, for_import, for_select));	// Write person
	DO_SUB_ACTION (treat_person (NULL, tm_write_table_suffix));

	return what_next::go_on ();
}
/*}}}*/

what_next write_message_list (list<sk_db::import_message> messages)/*{{{*/
	/*
	 * Write a list of messages concerning importing of master data.
	 * Paramters:
	 *   - messages: the list to write.
	 * Return value:
	 *   - wn_go_on on success.
	 *   - something else on error.
	 */
{
	DO_SUB_ACTION (treat_person (NULL, tm_write_table_prefix));
	DO_SUB_ACTION (treat_person (NULL, tm_write_table_header, "", "", NULL, true, false, "Problem"));

	list<sk_db::import_message>::const_iterator end=messages.end ();
	for (list<sk_db::import_message>::const_iterator it=messages.begin (); it!=end; ++it)
	{
		if ((*it).get_p1 ())
		{
			sk_person p=*((*it).get_p1 ());	// TODO remove when treat_person does not require a writeable object
			DO_SUB_ACTION (treat_person (&p, tm_write_table_data, "", "", NULL, true, false, (*it).description (false)));	// Write person
		}
		else
		{
			return what_next::output_error ("Fehlermeldung ohne Person (Programmfehler)");
		}
	}

	DO_SUB_ACTION (treat_person (NULL, tm_write_table_suffix));

	return what_next::go_on ();
}
/*}}}*/



// Field conversion
// Note: rather than continuing development of these functions, better
// implement the object_fields successor.
void sk_user_to_fields (const sk_user &user)/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Converts an sk_user structure to the (global) sk_user fields.
	 * Paramters:
	 *   - user: the user to convert.
	 * See also:
	 *   - sk_user_from_fields
	 */
{
	// TODO don't use global variable but pass reference
	// TODO replace object_field
	// TODO sk_user_{to,from}_fields are almost identical. Merge?
	// Iterate over all sk_user fields and, depending on the label, read the
	// data from the sk_user.
	list<object_field>::const_iterator end=fields_sk_user.end ();
	for (list<object_field>::iterator field=fields_sk_user.begin (); field!=end; ++field)
	{
		string label=(*field).get_label ();

		if (label==field_user_username)             (*field).set_to (user.username);
		else if (label==field_user_password)        {}
		else if (label==field_user_password_repeat) {}
		else if (label==field_user_club_admin)      (*field).set_to (user.perm_club_admin);
		else if (label==field_user_read_flight_db)  (*field).set_to (user.perm_read_flight_db);
		else if (label==field_user_club)            (*field).set_to (user.club);
		else if (label==field_user_person)          (*field).set_to (user.person);
		else
			throw ex_write_error_document ("Unbehandeltes Feld \""+(*field).get_caption ()+"\" in sk_user_to_fields").is_program_error ();
	}
}
/*}}}*/

void sk_user_from_fields (sk_user &user)/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Converts the (global) sk_user fields to an sk_user.
	 * Paramters:
	 *   - user: the user to write.
	 * See also:
	 *   - sk_user_to_fields
	 */
{
	// TODO don't use global variable but pass reference
	// TODO replace object_field
	// TODO sk_user_{to,from}_fields are almost identical. Merge?
	// Iterate over all sk_user fields and, depending on the label, read the
	// data from the sk_user.
	list<object_field>::const_iterator end=fields_sk_user.end ();
	for (list<object_field>::iterator field=fields_sk_user.begin (); field!=end; ++field)
	{
		string label=(*field).get_label ();

		if (label==field_user_username)             user.username=(*field).get_string ();
		else if (label==field_user_password)        {}
		else if (label==field_user_password_repeat) {}
		else if (label==field_user_club_admin)      user.perm_club_admin=(*field).get_bool ();
		else if (label==field_user_read_flight_db)  user.perm_read_flight_db=(*field).get_bool ();
		else if (label==field_user_club)            user.club=(*field).get_string ();
		else if (label==field_user_person)          user.person=(*field).get_db_id ();
		else
			throw ex_write_error_document ("Unbehandeltes Feld \""+(*field).get_caption ()+"\" in sk_user_from_fields").is_program_error ();
	}
}
/*}}}*/

void flugbuch_entry_to_fields (const flugbuch_entry &fbe, bool no_letters=false)/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Converts a flugbuch entry to the (global) flugbuch object_fields.
	 * Paremters:
	 *   - fbe: the entry to convert.
	 */
{
	// TODO don't use global variable but pass reference
	// TODO replace object_field
	// Iterate over all flugbuch_entry fields and, depending on the label, read
	// the data from the flugbuch_entry.
	list<object_field>::const_iterator end=fields_flugbuch_entry.end ();
	for (list<object_field>::iterator field=fields_flugbuch_entry.begin (); field!=end; ++field)
	{
		string label=(*field).get_label ();

		if (label==field_flugbuch_tag)                   (*field).set_to (fbe.tag_string ());
		else if (label==field_flugbuch_muster)           (*field).set_to (fbe.muster);
		else if (label==field_flugbuch_kennzeichen)      (*field).set_to (fbe.registration);
		else if (label==field_flugbuch_flugzeugfuehrer)  (*field).set_to (fbe.flugzeugfuehrer);
		else if (label==field_flugbuch_begleiter)        (*field).set_to (fbe.begleiter);
		else if (label==field_flugbuch_startart)         (*field).set_to (fbe.startart);
		else if (label==field_flugbuch_ort_start)        (*field).set_to (fbe.ort_start);
		else if (label==field_flugbuch_ort_landung)      (*field).set_to (fbe.ort_landung);
		else if (label==field_flugbuch_zeit_start)       (*field).set_to (fbe.zeit_start_string (no_letters));
		else if (label==field_flugbuch_zeit_landung)     (*field).set_to (fbe.zeit_landung_string (no_letters));
		else if (label==field_flugbuch_flugdauer)        (*field).set_to (fbe.flugdauer_string ());
		else if (label==field_flugbuch_bemerkung)        (*field).set_to (fbe.bemerkung);
		else
			throw ex_write_error_document ("Unbehandeltes Feld \""+(*field).get_caption ()+"\" in flugbuch_entry_to_fields").is_program_error ();

		// TODO should we handle .invalid here?
	}
}
/*}}}*/

void bordbuch_entry_to_fields (const bordbuch_entry &bbe, bool no_letters=false)/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Converts a blugbuch entry to the (global) blugbuch object_fields.
	 * Paremters:
	 *   - bbe: the entry to convert.
	 */
{
	// TODO don't use global variable but pass reference
	// TODO replace object_field
	// Iterate over all bordbuch_entry fields and, depending on the label, read
	// the data from the bordbuch_entry.
	list<object_field>::const_iterator end=fields_bordbuch_entry.end ();
	for (list<object_field>::iterator field=fields_bordbuch_entry.begin (); field!=end; ++field)
	{
		string label=(*field).get_label ();

		if (label==field_bordbuch_date)                 (*field).set_to (bbe.datum_string ());
		else if (label==field_bordbuch_club)            (*field).set_to (bbe.club);
		else if (label==field_bordbuch_registration)    (*field).set_to (bbe.registration);
		else if (label==field_bordbuch_plane_type)      (*field).set_to (bbe.flugzeug_typ);
		else if (label==field_bordbuch_name)            (*field).set_to (bbe.name);
		else if (label==field_bordbuch_num_persons)     (*field).set_to (bbe.insassen_string ());
		else if (label==field_bordbuch_place_from)      (*field).set_to (bbe.ort_von);
		else if (label==field_bordbuch_place_to)        (*field).set_to (bbe.ort_nach);
		else if (label==field_bordbuch_starttime)       (*field).set_to (bbe.zeit_start_string (no_letters));
		else if (label==field_bordbuch_landtime)        (*field).set_to (bbe.zeit_landung_string (no_letters));
		else if (label==field_bordbuch_num_landings)    (*field).set_to (bbe.anzahl_landungen_string ());
		else if (label==field_bordbuch_flight_time)     (*field).set_to (bbe.betriebsdauer_string ());
		else
			throw ex_write_error_document ("Unbehandeltes Feld \""+(*field).get_caption ()+"\" in bordbuch_entry_to_fields").is_program_error ();
	}
}
/*}}}*/

void flight_to_fields (list<object_field> &fields, const sk_flug &f, const sk_flug_data &flight_data, int &num, const string &none_text="")/*{{{*/
	throw (ex_write_error_document)
	/*
	 * Converts a flight list entry to a list of object_fields.
	 * Paremters: XXX
	 *   - fields: the field list to write.
	 *   - f: the entry to convert.
	 *   - num: a sequential number.
	 */
{
	string error_text="???";

	bool is_airtow=flight_data.towplane.given;

	// Make the display text for the flight_data items:
	//   - If the item is not given, use the text for "none"
	//   - If the item is given but not OK, use the text for "error"
	//   - If the item is OK, use the real text.
#define ITEM(NAME, FLIGHT_DATA_MEMBER, VALUE);	\
	string NAME;	\
	if (!flight_data.FLIGHT_DATA_MEMBER.given) NAME=none_text;	\
	else if (!flight_data.FLIGHT_DATA_MEMBER.ok) NAME=error_text;	\
	else NAME=flight_data.FLIGHT_DATA_MEMBER.data->VALUE;

	ITEM (sa_text,               startart, get_short_description ())

	ITEM (registration,          plane,    registration            )
	ITEM (plane_type,            plane,    typ                     )
	ITEM (plane_club,            plane,    club                    )

	ITEM (pilot_name,            pilot,    pdf_name ()             )
	ITEM (pilot_first_name,      pilot,    vorname                 )
	ITEM (pilot_last_name,       pilot,    nachname                )
	ITEM (pilot_club,            pilot,    club                    )
	ITEM (pilot_club_id,         pilot,    club_id                 )

	ITEM (copilot_name,          copilot,  pdf_name ()             )
	ITEM (copilot_first_name,    copilot,  vorname                 )
	ITEM (copilot_last_name,     copilot,  nachname                )
	ITEM (copilot_club,          copilot,  club                    )
	ITEM (copilot_club_id,       copilot,  club_id                 )

	ITEM (towplane_registration, towplane, registration            )
#undef ITEM

	// If the persons are not given, have a look at the incomplete names.
	if (!flight_data.pilot.given)
	{
		if (!f.pvn.empty () || !f.pnn.empty ())
		{
			pilot_name=f.unvollst_pilot_name ();
			pilot_first_name=f.pvn;
			pilot_last_name=f.pnn;
			pilot_club=error_text;
			pilot_club_id=error_text;
		}
	}

	if (!flight_data.copilot.given)
	{
		if (!f.bvn.empty () || !f.bnn.empty ())
		{
			copilot_name=f.unvollst_begleiter_name ();
			copilot_first_name=f.bvn;
			copilot_last_name=f.bnn;
			copilot_club=error_text;
			copilot_club_id=error_text;
		}
	}

	if (f.flugtyp==ft_gast_privat || f.flugtyp==ft_gast_extern)
	{
		copilot_name="(Gast)";
		copilot_first_name=none_text;
		copilot_last_name=none_text;
		copilot_club_id=none_text;
		copilot_club=none_text;
	}


	// Make the display text for items read directly from the flight.
#define CONDITIONAL(CONDITION, VALUE) ((CONDITION)?(VALUE):none_text)
	string starttime         =CONDITIONAL (starts_here (f.modus) && f.gestartet,                                       f.startzeit.table_string (tz_utc, true, true)                 );
	string landtime          =CONDITIONAL (lands_here (f.modus) && f.gelandet,                                         f.landezeit.table_string (tz_utc, true, true)                 );
	string duration          =CONDITIONAL (starts_here (f.modus) && lands_here (f.modus) && f.gestartet && f.gelandet, f.flugdauer ().table_string (tz_timespan)                     );
	string landtime_towplane =CONDITIONAL (is_airtow && f.gestartet && lands_here (f.modus_sfz) && f.sfz_gelandet,     f.landezeit_schleppflugzeug.table_string (tz_utc, true, true) );
	string duration_towplane =CONDITIONAL (is_airtow && f.gestartet && lands_here (f.modus_sfz) && f.sfz_gelandet,     f.schleppflugdauer ().table_string (tz_timespan)              );
	string zielort_towplane  =CONDITIONAL (is_airtow,                                                                  f.zielort_sfz                                                 );
	string mode_towplane     =CONDITIONAL (is_airtow,                                                                  modus_string (f.modus_sfz, ls_csv)                            );
#undef CONDITION

	// Another towplane special: the towplane comment is added if it is not
	// explicitly known from the startart.
	string towplane_comment;
	if (is_airtow && flight_data.towplane.ok && !flight_data.startart.data->towplane_known ())
		towplane_comment=flight_data.towplane.data->registration;

	// Iterate over all fields and, depending on the label, read the data from
	// the flight and write it to the field.
	// Depending on the list used, not all of these fields are set.
	list<object_field>::const_iterator end=fields.end ();
	for (list<object_field>::iterator field=fields.begin (); field!=end; ++field)
	{
		string label=(*field).get_label ();

		if (label==field_flight_number)                 (*field).set_to (num_to_string (num));
		else if (label==field_flight_registration)      (*field).set_to (registration);
		else if (label==field_flight_type)              (*field).set_to (plane_type);
		else if (label==field_flight_pilot)             (*field).set_to (pilot_name);
		else if (label==field_flight_copilot)           (*field).set_to (copilot_name);
		else if (label==field_flight_club)              (*field).set_to (pilot_club);
		else if (label==field_flight_startart)          (*field).set_to (sa_text);
		else if (label==field_flight_starttime)         (*field).set_to (starttime);
		else if (label==field_flight_landtime)          (*field).set_to (landtime);
		else if (label==field_flight_duration)          (*field).set_to (duration);
		else if (label==field_flight_landtime_towplane) (*field).set_to (landtime_towplane);
		else if (label==field_flight_duration_towplane) (*field).set_to (duration_towplane);
		else if (label==field_flight_num_landings)      (*field).set_to (num_to_string (f.landungen));
		else if (label==field_flight_startort)          (*field).set_to (f.startort);
		else if (label==field_flight_zielort)           (*field).set_to (f.zielort);
		else if (label==field_flight_zielort_towplane)  (*field).set_to (zielort_towplane);
		else if (label==field_flight_comments)          (*field).set_to (concatenate_comments (towplane_comment, f.bemerkungen));
		// Additional fields not used in flightlist
		else if (label==field_flight_date)                  (*field).set_to (q2std (f.effdatum ().toString ("yyyy-MM-dd")));
		else if (label==field_flight_plane_club)            (*field).set_to (plane_club);
		else if (label==field_flight_pilot_last_name)       (*field).set_to (pilot_last_name);
		else if (label==field_flight_pilot_first_name)      (*field).set_to (pilot_first_name);
		else if (label==field_flight_pilot_club)            (*field).set_to (pilot_club);
		else if (label==field_flight_pilot_club_id)         (*field).set_to (pilot_club_id);
		else if (label==field_flight_copilot_last_name)     (*field).set_to (copilot_last_name);
		else if (label==field_flight_copilot_first_name)    (*field).set_to (copilot_first_name);
		else if (label==field_flight_copilot_club)          (*field).set_to (copilot_club);
		else if (label==field_flight_copilot_club_id)       (*field).set_to (copilot_club_id);
		else if (label==field_flight_flight_type)           (*field).set_to (flugtyp_string (f.flugtyp, ls_csv));
		else if (label==field_flight_mode)                  (*field).set_to (modus_string (f.modus, ls_csv));
		else if (label==field_flight_registration_towplane) (*field).set_to (towplane_registration);
		else if (label==field_flight_mode_towplane)         (*field).set_to (mode_towplane);
		else if (label==field_flight_abrechnungshinweis)    (*field).set_to (f.abrechnungshinweis);
		else if (label==field_flight_id)                    (*field).set_to (num_to_string (f.id));
		else
			throw ex_write_error_document ("Unbehandeltes Feld \""+(*field).get_caption ()+"\" in flight_to_fields").is_program_error ();
	}
}
/*}}}*/


void write_flugbuch (latex_document &ldoc, const QPtrList<flugbuch_entry> &flugbuch, const string &date_text, const string &person_name)/*{{{*/
	// XXX
{
	// TODO code duplication with csv writing
	setup_latex_headings (ldoc, "Flugbuch f�r "+latex_escape (person_name), date_text);
	ldoc.landscape=false;

	if (flugbuch.isEmpty ())
	{
		ldoc.write_text ("Keine Fl�ge\n");
	}
	else
	{
		table tab;

		// Make a header row
		table_row header_row=table_row_from_fields (fields_flugbuch_entry, true);

		// For each bordbuch entry, make a table row.
		for (QPtrListIterator<flugbuch_entry> it (flugbuch); *it; ++it)
		{
			flugbuch_entry_to_fields (**it);
			tab.push_back (table_row_from_fields (fields_flugbuch_entry));
		}

		ldoc.write (tab, header_row, widths_flugbuch_entry);
	}
}
/*}}}*/

void write_bordbuch (latex_document &ldoc, const QPtrList<bordbuch_entry> &bordbuch, const string &date_text)/*{{{*/
	// XXX
{
	// TODO code duplication with csv writing
	setup_latex_headings (ldoc, "Bordb�cher "+latex_escape (opts.ort), date_text);
	ldoc.landscape=false;

	if (bordbuch.isEmpty ())
	{
		ldoc.write_text ("Keine Fl�ge\n");
	}
	else
	{
		table tab;

		// Make a header row
		table_row header_row=table_row_from_fields (fields_bordbuch_entry, true);

		// For each bordbuch entry, make a table row.
		for (QPtrListIterator<bordbuch_entry> it (bordbuch); *it; ++it)
		{
			bordbuch_entry_to_fields (**it);
			tab.push_back (table_row_from_fields (fields_bordbuch_entry));
		}

		ldoc.write (tab, header_row, widths_bordbuch_entry);
	}
}
/*}}}*/

void write_flightlist (latex_document &ldoc, const flight_list &flights, const string &date_text)/*{{{*/
//	// XXX
{
	// TODO code duplication with csv
	setup_latex_headings (ldoc, "Hauptflugbuch "+latex_escape (opts.ort), date_text);

	ldoc.landscape=true;

	if (flights.isEmpty ())
	{
		ldoc.write_text ("Keine Fl�ge\n");
	}
	else
	{
		table tab;

		// Make a header row
		table_row header_row=table_row_from_fields (fields_flightlist_entry, true);

		// For each flight, make a table row.
		int num=0;
		for (QPtrListIterator<sk_flug> it (flights); *it; ++it)
		{
			num++;

			sk_flug_data flight_data=sk_flug_data::owner ();
			db.make_flight_data (flight_data, **it);

			flight_to_fields (fields_flightlist_entry, **it, flight_data, num, "---");
			tab.push_back (table_row_from_fields (fields_flightlist_entry));
		}

		ldoc.write (tab, header_row, widths_flightlist);
	}
}
/*}}}*/


// Specific field handling
void field_user_lock (list<object_field> &fields)/*{{{*/
{
	list<object_field>::const_iterator fields_end=fields.end ();
	for (list<object_field>::iterator field_it=fields.begin (); field_it!=fields_end; ++field_it)
	{
		if (session_access==dba_sk_user)
		{
			// Club admins can only create users for their own club.
			if ((*field_it).get_label ()==field_user_club)
			{
				(*field_it).parse_text (session_user->club);
				(*field_it).set_locked (true);
			}
			else if ((*field_it).get_label ()==field_user_club_admin)
				(*field_it).set_locked (true);
			else if ((*field_it).get_label ()==field_user_read_flight_db)
				(*field_it).set_locked (true);
			else
			{
				(*field_it).set_locked (false);
			}
		}
		else
		{
			(*field_it).set_locked (false);
		}
	}
}
/*}}}*/


// Generic field handling
void write_fields_display (list<object_field> fields)/*{{{*/
{
	html_table table;

	list<object_field>::const_iterator end=fields.end ();
	for (list<object_field>::const_iterator it=fields.begin (); it!=end; ++it)
	{
		if ((*it).get_list_display ())
		{
			html_table_row row;
			row.push_back (html_table_cell::text ((*it).get_caption ()+":"));
			row.push_back (html_table_cell::text ((*it).make_display_text ()));

			table.push_back (row);
		}
	}

	document.write (table, true);
}
/*}}}*/

void fields_write_edit_table (const list<object_field> &fields, bool create_new, const string &ident_arg, const string &ident_val)/*{{{*/
{
	// For each field, write the table row
	// TODO html_table verwenden
	document.start_tag ("table");
	list<object_field>::const_iterator fields_end=fields.end ();
	for (list<object_field>::const_iterator field_it=fields.begin (); field_it!=fields_end; ++field_it)
	{
		const object_field &field=*field_it;

		bool field_present                =create_new? field.create_present ()          : field.edit_present ();
		bool field_display                =create_new? field.get_create_display ()      : field.get_edit_display ();
		bool field_edit                   =create_new? field.get_create_edit ()         : field.get_edit_edit ();
		const string &field_state         =create_new? field.get_create_state ()        : field.get_edit_state ();
		const string &field_state_caption =create_new? field.get_create_state_caption (): field.get_edit_state_caption ();

		if (field_present)
		{
			// Start a row
			document.start_tag ("tr");

			// Write the caption cell
			document.write_table_data (html_escape (field.get_caption ()+":"));

			// Start the value/edit cell
			document.start_tag ("td");

			if (field_display)
				document.write (html_escape (field.make_display_text ())).endl ();

			if (field_edit)
			{
				if (field.get_locked ())
				{
					document.write_text (field.make_display_text ());
				}
				// TODO auslagern dieses switches
				else switch (field.get_data_type ())
				{
					case object_field::dt_string:
						document.write_input_text (field.get_label (), field.get_string ());
						break;
					case object_field::dt_bool:
						// Problem with checkbox is that it is not possible to
						// distinguish "off" and "not given".
						document.write_input_select_bool (field.get_label (), field.get_bool ());
						break;
					case object_field::dt_db_id:
						document.write_input_text (field.get_label (), field.make_text ());
						break;
					case object_field::dt_password:
						document.write_input_password (field.get_label (), field.make_text ());
						break;
					case object_field::dt_special:
						break;
				}
			}

			if (!field_edit || field.get_locked ())
				document.write_hidden_field (field.get_label (), field.make_text ());

			if (!field_state.empty ())
			{
				// The field can be edited using a link/button
				argument_list args;
				args.set_value (ident_arg, ident_val);

				if (field_edit)
				{
					// The field can also be edited directly. This means that
					// the editing state is some kind of "select" state which
					// also preserves the information entered so far. We must
					// enter such a state using a button because we must post
					// the information entered for saving it in the session.
					// We use the regular do_action state and set a flag with
					// this button. Another possibility would be to enter the
					// select state directly (which would have the additional
					// advantage that the cgi_new_state field could be passed
					// as value of the button instead of using a hidden field.
					// However, in this case we would need to check  where we
					// came from in the select state.

					document
						.write_text_button (field_state, bool_to_string (true), field_state_caption);
				}
				else
				{
					// This field cannot be edited directly. This means that
					// an ordinary link to another state is used for editing.
					// This is to be changed entually because is has the dis-
					// advantage that the other changes made by the user are
					// lost.
					document
						.write ("(")
						.write_text_link (back_link_url (field_state, args), field_state_caption)
						.write (")")
						;
				}
			}

			document
				.end_tag ("td")
				.end_tag ("tr");
		}
	}

	// The submit button.
	document
		.start_tag ("tr")
		.start_tag ("td", "colspan=2")
		.write_submit (create_new?"Anlegen":"�ndern")
		.end_tag ("td")
		.end_tag ("tr")
		;

	document.end_tag ("table");
}
/*}}}*/

void fields_from_cgi (list<object_field> &fields)/*{{{*/
{
	list<object_field>::const_iterator end=fields.end ();
	for (list<object_field>::iterator field=fields.begin (); field!=end; ++field)
	{
		const string &label=(*field).get_label ();
		if (CGI_HAS_F (label))
		{
			(*field).parse_text (CGI_READ_F (label));
			(*field).set_given (true);

			// If a field is given although it is locked, beat the
			// user.
			if ((*field).get_locked ())
				if ((*field).make_text ()!=CGI_READ_F (label))
					throw ex_write_error_document ("Gesperrtes Feld \""+(*field).get_caption ()+"\" angegeben (ist: \""+CGI_READ_F (label)+"\"; soll: \""+(*field).make_text_safe ()+"\")");
		}
		else
		{
			// Don't clear the field because we might still need the
			// values set by the lock function and we can still look
			// at given to see if the field was present.
			(*field).set_given (false);
		}
	}
}
/*}}}*/

void fields_write_edit_form (const list<object_field> &fields, const string &url, const string &next_state, bool create_new, const string &ident_arg, const string &ident_val)/*{{{*/
{
	// TODO auslagern
	document.start_tag ("form", "action=\""+url+"\" method=\"POST\"");

	fields_write_edit_table (fields, create_new, ident_arg, ident_val);

	// We need to tell the next state about the username, in addition to the
	// standard stuff (session_id, debug, etc.)
	argument_list additional_args;
	additional_args.set_value (ident_arg, ident_val);
	if (create_new) additional_args.set_value (arg_cgi_create_new);
	document
		.write (back_form_hidden (next_state))	// TODO make back_form_hidden return a list for write_hidden_fields
		.write_hidden_fields (additional_args)
		;

	document.end_tag ("form");
}
/*}}}*/

void fields_write_to_session (const list<object_field> &fields)/*{{{*/
{
	list<object_field>::const_iterator end=fields.end ();
	for (list<object_field>::const_iterator field=fields.begin (); field!=end; ++field)
	{
		const string &label=(*field).get_label ();
		const string text=(*field).make_text ();
		session.args.set_value (label, text);
	}
}
/*}}}*/

void fields_read_from_session (list<object_field> &fields)/*{{{*/
{
	list<object_field>::const_iterator end=fields.end ();
	for (list<object_field>::iterator field=fields.begin (); field!=end; ++field)
	{
		const string &label=(*field).get_label ();
		if (session.args.has_argument (label))
		{
			(*field).parse_text (session.args.get_value (label));
//			(*field).set_given (true);
		}
//		else
//		{
//			(*field).set_given (false);
//		}
	}
}
/*}}}*/

void fields_delete_from_session (const list<object_field> &fields)/*{{{*/
{
	list<object_field>::const_iterator end=fields.end ();
	for (list<object_field>::const_iterator field=fields.begin (); field!=end; ++field)
	{
		const string &label=(*field).get_label ();
		session.args.remove (label);
	}
}
/*}}}*/


// State handler functions _STATE_
what_next handler_login ()/*{{{*/
{
	html_table table;
	html_table_row row;

	row.clear ();
	row.push_back (html_table_cell::text ("Name:"));
	row.push_back (html_table_cell (document.make_input_text (arg_cgi_username, CGI_READ (username))));
	table.push_back (row);

	row.clear ();
	row.push_back (html_table_cell::text ("Password:"));
	row.push_back (html_table_cell (document.make_input_password (arg_cgi_password)));
	table.push_back (row);

	row.clear ();
	row.push_back (html_table_cell (document.make_input_checkbox (arg_cgi_debug, "Debug", CGI_HAS (debug)), false, 2));
	table.push_back (row);

	if (CGI_HAS (debug))
	{
		row.clear ();
		row.push_back (html_table_cell (document.make_input_checkbox (arg_cgi_no_redirects, "Keine redirects", CGI_HAS (no_redirects)), false, 2));
		table.push_back (row);
	}

	row.clear ();
	// TODO: use the regular submit button and a hidden field, like everyone
	// else. But don't use back_form hidden because it overwrites debug.
//	row.push_back (html_table_cell::text (document.make_submit ("Anmelden"), false, 2));
	row.push_back (html_table_cell (document.make_text_button (arg_cgi_new_state, web_do_login, "Anmelden"), false, 2));
	table.push_back (row);

	document
		.start_tag ("form", "action=\""+relative_url+"\" method=\"POST\"")
		.write (table)
//		.write_hidden_field (arg_cgi_debug)
		.write_hidden_field (arg_cgi_new_state, web_do_login)
//		.write (back_form_hidden (web_do_login))
		.end_tag ("form")
		;

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_login ()/*{{{*/
{
	// We may only do this via POST requests.
	if (request_method!=rm_post)
		return what_next::output_error ("Aus Sicherheitsgr�nden ist dieser Zugang nicht m�glich. Bitte die Anmeldeseite verwenden.");

	// Check parameter: username. Must be present and non-empty.
	if (!CGI_HAS (username)) return what_next::go_to_state (web_login, "Kein Benutzername angegeben", true);
	string username=CGI_READ (username);
	if (username.empty ()) return what_next::go_to_state (web_login, "Kein Benutzername angegeben", true);
	if (!user_name_valid (username)) return what_next::go_to_state (web_login, "Benutzername enth�lt ung�ltige Zeichen", true);

	// Check parameter password. Must be present and non-empty.
	if (!CGI_HAS (password)) return what_next::go_to_state (web_login, "Kein Passwort angegeben", true);
	string password=CGI_READ (password);
	// Password empty is not an error, although it is for setting new
	// passwords.

	// Try to authenticate the user using the username and password specified.
	string error_message;
	if (authenticate (username, password, error_message))
	{
		// Authentication succeeded. Try to create a session.
		session=web_session::create ();
		if (session.is_ok ())
		{
			// Session created OK. Save the username (and possibly password) to
			// the session and redirect to the main menu (redirect because this
			// was loaded by POST).
			SESSION_WRITE (login_name, username);
			if (determine_user_class (username)==uc_mysql_user) SESSION_WRITE (password, password);

			// Save the remote address to the session for later checking.
			if (remote_address.empty ()) return what_next::output_error ("remote_address ist leer (Programmfehler)");
			SESSION_WRITE (remote_address, remote_address);
			return make_redirect (web_main_menu);
		}
		else
		{
			// Creating a session failed. Display the error.
			return what_next::output_error ("Anlegen einer Sitzung fehlgeschlagen: "+session.get_error_description ());
		}
	}
	else
	{
		// Authentication failed. Display the error and go back to the login
		// state.
		return what_next::go_to_state (web_login, error_message, true);
	}
}
/*}}}*/


bool state_available (const string &label)/*{{{*/
{
	const web_interface_state &state=web_interface_state::from_list (label);

	// Login/logout are special
	if (label==web_login) return (!session.is_ok ());
	if (label==web_logout) return (session.is_ok ());

	if (!session.is_ok ())
	{
		// Local and anonomous states may be allowed nevertheless.
		if (state.get_allow_anon ()) return true;
		if (state.get_allow_local () && client_is_local) return true;

		// Other states are only allowed with session.
		return false;
	}

	if (!provides (session_access, state.get_db_access_needed ())) return false;

	return true;
}
/*}}}*/

string make_state_link (const string &state_label, const string &title="")/*{{{*/
{
	string link_title;
	if (!title.empty ())
	{
		link_title=title;
	}
	else try
	{
		const web_interface_state &state=web_interface_state::from_list (state_label);
		link_title=state.make_caption ();
	}
	catch (web_interface_state::ex_not_found)
	{
		link_title="???";
	}

	if (debug_enabled) link_title.append (" ("+state_label+")");

	return document.text_link (back_link_url (state_label), link_title);
}
/*}}}*/

void add_state_link_if_available (list<string> &link_list, const string &state_label, bool force=false)/*{{{*/
{
	if (force || debug_enabled || state_available (state_label))
		link_list.push_back (make_state_link (state_label));
}
/*}}}*/



what_next handler_main_menu ()/*{{{*/
{
	// TODO: hier eine Funktion, die einen korrekt titulierten Link auf den Zustand ausgibt.
	// TODO: consider only listing entries which are acually available

	list<string> database_list;
	list<string> account_list;
	list<string> person_list;
	list<string> user_list;

	add_state_link_if_available (database_list, web_flightlist);
	add_state_link_if_available (database_list, web_plane_logbook);
	add_state_link_if_available (database_list, web_person_logbook);
	add_state_link_if_available (database_list, web_flight_db);

	add_state_link_if_available (person_list, web_list_persons);
	add_state_link_if_available (person_list, web_create_person);
	add_state_link_if_available (person_list, web_master_data_import);

	add_state_link_if_available (user_list, web_user_list);
	add_state_link_if_available (user_list, web_user_add);

	add_state_link_if_available (account_list, web_login);
	add_state_link_if_available (account_list, web_change_password);
	add_state_link_if_available (account_list, web_logout);

	list<string> main_list;
	if (!database_list.empty ()) main_list.push_back ("Datenbankzugriff\n"+document.make_list (database_list));
	if (!person_list.empty ()) main_list.push_back ("Personenverwaltung\n"+document.make_list (person_list));
	if (!user_list.empty ()) main_list.push_back ("Benutzerverwaltung\n"+document.make_list (user_list));
	if (!account_list.empty ()) main_list.push_back ("Account\n"+document.make_list (account_list));

	if (debug_enabled)
	{
		list<string> debug_list;
		argument_list arg_url;
		arg_url.set_value (arg_cgi_url, "http://www.google.de");
		debug_list.push_back (document.text_link (back_link_url (web_test_redirect), "Redirect testen"));
		debug_list.push_back (document.text_link (back_link_url (web_test_redirect, arg_url), "Redirect testen (mit URL)"));
		main_list.push_back ("Debug\n"+document.make_list (debug_list));
	}

	document.write_list (main_list);

	return what_next::output_document ();
}
/*}}}*/

what_next handler_logout ()/*{{{*/
{
	if (session.is_ok ()) session.destroy ();
	return what_next::do_redirect (absolute_url);
}
/*}}}*/

what_next handler_change_password ()/*{{{*/
{
	switch (determine_user_class (SESSION_READ (login_name)))
	{
		case uc_none:
			return what_next::output_error ("Passwort�nderung f�r Benutzer der Klasse uc_none nicht m�glich");
			break;
		case uc_mysql_user:
			return what_next::output_error ("Passwort�nderung f�r Benutzer der Klasse uc_mysql_user nicht implementiert");
			// SET PASSWORD=PASSWORD ('foo');
			break;
		case uc_sk_user:
		{
			document.write_paragraph ("Passwort�nderung f�r Benutzer "+html_escape (SESSION_READ (login_name)));
			// TODO use document methods
			document.write (
				"<form action=\""+relative_url+"\" method=\"POST\">\n"
				"  <table>\n"
				"    <tr>\n"
				"      <td>Altes Passwort:</td>\n"
				"      <td><input type=\"password\" name=\""+arg_cgi_old_password+"\"></td>\n"
				"    </tr>\n"
				"    <tr>\n"
				"      <td>Neues Passwort:</td>\n"
				"      <td><input type=\"password\" name=\""+arg_cgi_new_password_1+"\"></td>\n"
				"    </tr>\n"
				"    <tr>\n"
				"      <td>Neues Passwort (wiederholen):</td>\n"
				"      <td><input type=\"password\" name=\""+arg_cgi_new_password_2+"\"></td>\n"
				"    </tr>\n"
				"    <tr><td colspan=2><input type=\"submit\" value=\"�ndern\"></td></tr>\n"
				"  </table>\n"
				+back_form_hidden (web_do_change_password)+
				"</form>\n"
				);
		} break;
	}

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_change_password ()/*{{{*/
{
	string user_name=SESSION_READ (login_name);
	string old_password=CGI_READ (old_password);
	string new_password_1=CGI_READ (new_password_1);
	string new_password_2=CGI_READ (new_password_2);

	check_username (user_name);

	switch (determine_user_class (user_name))
	{
		case uc_none:
			return what_next::output_error ("Passwort�nderung f�r Benutzer der Klasse uc_none nicht m�glich");
			break;
		case uc_mysql_user:
			return what_next::output_error ("Passwort�nderung f�r Benutzer der Klasse uc_mysql_user nicht implementiert");
			// dann auch gespeichertes passwort (in session) aktualisieren
			break;
		case uc_sk_user:
		{
			if (user_name.empty ()) return what_next::output_error ("login_name nicht gesetzt");
			if (new_password_1!=new_password_2) return what_next::go_to_state (web_change_password, "Neue Passw�rter stimmen nicht �berein", true);
			if (new_password_1.empty ()) return what_next::go_to_state (web_change_password, "Neues Passwort nicht angegeben", true);
			string error_message;
			if (!authenticate (user_name, old_password, error_message)) return what_next::go_to_state (web_change_password, error_message, true);

			int ret=db.sk_user_change_password (user_name, new_password_1);
			CHECK_DB_ERROR_ERROR;

			return redirect_to_result ("Passwort�nderung erfolgt", false, web_main_menu);
		} break;
	}

	return what_next::output_error ("Unbehandelte Benutzerklasse in handler_do_change_password (Programmfehler)");
}
/*}}}*/

what_next handler_list_persons ()/*{{{*/
{
	document.write_paragraph (document.text_link (back_link_url (web_create_person), "Neu anlegen"));

	QPtrList<sk_person> persons; persons.setAutoDelete (true);
	int ret=db.list_persons_all (persons);
	CHECK_DB_ERROR_ERROR;

	DO_SUB_ACTION (write_person_list (persons, false, web_display_person, arg_cgi_id));

	document.write_paragraph (num_to_string (persons.count ())+" Personen");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_edit_person ()/*{{{*/
{
	require_club_admin ();

	db_id id=atoi (CGI_READ (id).c_str ());
	CHECK_ID;

	sk_person person;
	int ret=db.get_person (&person, id);
	CHECK_DB_ERROR_ERROR;

	DO_SUB_ACTION (treat_person (&person, tm_write_edit));

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_edit_person ()/*{{{*/
{
	require_club_admin ();
	// We don't require a specific club because there may be the need to edit
	// persons from external clubs.

	// Get the ID
	db_id id=atoi (CGI_READ (id).c_str ());
	CHECK_ID;

	// Read the person
	sk_person person;
	int ret=db.get_person (&person, id);
	CHECK_DB_ERROR_ERROR;

	bool old_editable=person.editierbar;

	// Modify the person
	DO_SUB_ACTION (treat_person (&person, tm_edit));

	// If the club ID is not empty, check if there are other persons with this club ID
	if (!person.club_id.empty ())
	{
		QPtrList<sk_person> persons; persons.setAutoDelete (true);
		db.list_persons_by_club_club_id (persons, person.club, person.club_id);
		// If there is at least one persons with a different ID and the same club
		// ID, then this is an error.
		bool duplicate_club_id=false;
		for (QPtrListIterator<sk_person> it (persons); *it; ++it)
			if ((*it)->id!=person.id) duplicate_club_id=true;

		if (duplicate_club_id) return what_next::output_error ("Vereins-ID nicht eindeutig");
	}


	// Write the person
	db_id ret_id=db.write_person (&person);
	if (id_invalid (ret_id)) return what_next::output_error ("Aktualisieren fehlgeschlagen");

	// If the "editable" state was changed (i. e., it is different from the one
	// in the database), change it.
	try
	{
		if (person.editierbar!=old_editable)
			db.make_person_editable (ret_id, person.editierbar);
	}
	catch (sk_db::ex_operation_failed &e)
	{
		return what_next::output_error (e.description (true));
	}
	catch (sk_db::ex_legacy_error &e)
	{
		return what_next::output_error (e.description (true));
	}
	catch (sk_exception &e)
	{
		return what_next::output_error (e.description ());
	}

	return make_redirect (web_list_persons, make_anchor_name (id));
}
/*}}}*/

what_next handler_display_person ()/*{{{*/
{
	db_id id=atoi (CGI_READ (id).c_str ());
	CHECK_ID;

	// Read the person
	sk_person person;
	int ret=db.get_person (&person, id);
	CHECK_DB_ERROR_ERROR;

	// Display the person
	DO_SUB_ACTION (treat_person (&person, tm_write_display));

	// TODO Display flights of the person

	return what_next::output_document ();
}
/*}}}*/

what_next handler_delete_person ()/*{{{*/
{
	require_club_admin ();

	db_id id=atoi (CGI_READ (id).c_str ());
	CHECK_ID;

	if (db.person_used (id))
		return what_next::output_error ("Die Person kann nicht gel�scht werden, weil sie noch verwendet wird.");
		// TODO where?

	// Read the person
	sk_person person;
	int ret=db.get_person (&person, id);
	CHECK_DB_ERROR_ERROR;

	document.write_paragraph ("Die folgende Person wird gel�scht:");

	// Display the person
	document.start_paragraph ();
	DO_SUB_ACTION (treat_person (&person, tm_write_display));
	document.end_paragraph ();

	// TODO: nicht manuell
	argument_list delete_args;
	delete_args.set_value (arg_cgi_id, num_to_string (person.id));
	document
		.write_paragraph ("OK?")
		.start_paragraph ()
		.write_text_link (back_link_url (web_do_delete_person, delete_args), "Wirklich l�schen")
		.write (" ")
		.write_text_link (back_link_url (web_list_persons, make_anchor_name (person.id)), "Zur�ck")
		.end_paragraph ()
		;

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_delete_person ()/*{{{*/
{
	int ret;
	require_club_admin ();
	// We don't require a specific club because there may be the need to edit
	// persons from external clubs.

	db_id id=atoi (CGI_READ (id).c_str ());
	CHECK_ID;

	if (db.person_used (id))
		return what_next::output_error ("Die Person kann nicht gel�scht werden, weil sie noch verwendet wird.");
		// TODO where?

	// Delete
	ret=db.delete_person (id);
	CHECK_DB_ERROR_ERROR;

	return make_redirect (web_list_persons);
}
/*}}}*/

what_next handler_result ()/*{{{*/
	// session arguments: message_text, message_error
{
	string message_text=SESSION_READ (result_text);
	bool message_error=SESSION_HAS (result_error);
	string next_state=SESSION_READ (result_state);

	if (message_error)
		document.write_error_paragraph (message_text);
	else
		document.write_paragraph (message_text);

	if (!next_state.empty ())
	{
		try
		{
			web_interface_state state=web_interface_state::from_list (next_state);
			document.write_text_link (back_link_url (state.get_label ()), state.make_caption ());
		}
		catch (web_interface_state::ex_not_found)
		{
			document.write_paragraph ("(Ung�litiger Zustand "+next_state+")");
		}
	}

	return what_next::output_document ();
}
/*}}}*/

what_next handler_create_person ()/*{{{*/
{
	require_club_admin ();
	// We don't require a specific club because there may be the need to edit
	// persons from external clubs.

	DO_SUB_ACTION (treat_person (NULL, tm_write_create));
	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_create_person ()/*{{{*/
{
	require_club_admin ();

	// TODO code duplication with handler_do_edit_person

	// Create a new person
	sk_person person;

	// TODO Verein des Benutzers als default eintragen
	// Modify the person
	DO_SUB_ACTION (treat_person (&person, tm_create));

	// TODO code duplication with handler_do_edit_person
	// If the club ID is not empty, check if there are other persons with this
	// club ID
	if (!person.club_id.empty ())
	{
		QPtrList<sk_person> persons; persons.setAutoDelete (true);
		db.list_persons_by_club_club_id (persons, person.club, person.club_id);
		// If there is at least one persons with a different ID, then this is an
		// error.
		bool duplicate_club_id=false;
		for (QPtrListIterator<sk_person> it (persons); *it; ++it)
			if ((*it)->id!=person.id) duplicate_club_id=true;

		if (duplicate_club_id) return what_next::output_error ("Vereins-ID nicht eindeutig");
	}

	// Write the person
	db_id id=db.write_person (&person);
	if (id_invalid (id)) return what_next::output_error ("Anlegen fehlgeschlagen");

	return make_redirect (web_list_persons, make_anchor_name (id));
}
/*}}}*/

what_next handler_select_merge_person ()/*{{{*/
{
	int ret;
	require_club_admin ();

	db_id id=atoi (CGI_READ (id).c_str ());
	CHECK_ID;

	sk_person person;
	ret=db.get_person (&person, id);
	CHECK_DB_ERROR_ERROR;

	document.write_paragraph ("Die folgende (falsche) Person wird �berschrieben:");
	DO_SUB_ACTION (treat_person (&person, tm_write_display));

	document.write_paragraph ("Bitte die korrekte Person ausw�hlen:");

	// For linking to the do_merge state, we need to pass the wrong person.
	argument_list additional_args;
	additional_args.set_value (arg_cgi_id, num_to_string (person.id));

	QPtrList<sk_person> persons; persons.setAutoDelete (true);
	ret=db.list_persons_all (persons);
	CHECK_DB_ERROR_ERROR;
	DO_SUB_ACTION (write_person_list (persons, false, web_merge_person, arg_cgi_correct_person, &additional_args, false, true));

	document.write_paragraph (document.text_link (back_link_url (web_list_persons, make_anchor_name (person.id)), "Zur�ck zur Benutzerliste"));

	return what_next::output_document ();
}
/*}}}*/

what_next handler_merge_person ()/*{{{*/
{
	int ret;

	require_club_admin ();

	db_id wrong_id=atoi (CGI_READ (id).c_str ());
	db_id correct_id=atoi (CGI_READ (correct_person).c_str ());
	CHECK_AN_ID (wrong_id, id);
	CHECK_AN_ID (correct_id, correct_person);

	if (correct_id==wrong_id) return what_next::go_to_state (web_select_merge_person, "Identische Person ausgew�hlt", true);

	// Write persons and get confirmation

	// Get the persons from the database
	// Wrong person
	sk_person wrong_person;
	ret=db.get_person (&wrong_person, wrong_id);
	CHECK_DB_ERROR_ERROR;
	// Correct person
	sk_person correct_person;
	ret=db.get_person (&correct_person, correct_id);
	CHECK_DB_ERROR_ERROR;

	document.write_paragraph ("Die folgende (falsche) Person:");
	DO_SUB_ACTION (treat_person (&wrong_person, tm_write_display));

	document.write_paragraph ("wird durch folgende (korrekten) Person �berschrieben:");
	DO_SUB_ACTION (treat_person (&correct_person, tm_write_display));

	document.write_paragraph ("OK?");

	// OK, merge
	argument_list merge_args;
	merge_args.set_value (arg_cgi_id, num_to_string (wrong_id));
	merge_args.set_value (arg_cgi_correct_person, num_to_string (correct_person.id));
	string merge_link_target=back_link_url (web_do_merge_person, merge_args);	//, "Neu ausw�hlen";

	// Back to selection
	argument_list back_select_args;
	back_select_args.set_value (arg_cgi_id, num_to_string (wrong_id));
	string back_select_link_target=back_link_url (web_select_merge_person, back_select_args); //, "Neu ausw�hlen");

	// Back to list
	string anchor=make_anchor_name (wrong_person.id);
	string back_list_link_target=back_link_url (web_list_persons, anchor);	//, "Zur�ck zur Personenliste");

	document
		.start_paragraph ()
		.write_text_link (merge_link_target, "�berschreiben")
		.write (" ")
		.write_text_link (back_select_link_target, "Neu ausw�hlen")
		.write (" ")
		.write_text_link (back_list_link_target, "Zur�ck zur Personenliste")
		.end_paragraph ()
		;

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_merge_person ()/*{{{*/
{
	require_club_admin ();

	db_id wrong_id=atoi (CGI_READ (id).c_str ());
	db_id correct_id=atoi (CGI_READ (correct_person).c_str ());
	CHECK_AN_ID (wrong_id, id);
	CHECK_AN_ID (correct_id, correct_person);

	if (correct_id==wrong_id) return what_next::go_to_state (web_select_merge_person, "Identische Person ausgew�hlt", true);

	try
	{
		db.merge_person (correct_id, wrong_id);
	}
	catch (sk_db::ex_operation_failed &e)
	{
		return what_next::output_error (e.description (true));
	}
	catch (sk_exception &e)
	{
		return what_next::output_error (e.description ());
	}

	SESSION_WRITE (one_time_message, "Person �berschrieben");
	return make_redirect (web_list_persons, make_anchor_name (correct_id));
}
///*}}}*/

// Contrary to the person_* handlers, the user_* handlers don't use treat_*
// functions but object_field. The object_field concept is newer and cleaner,
// although I will admit that the functions making use of it are still longer.
// Another difference is that the user_* handlers have a common function for
// editing and creating. It is unknown whether is is an advantage.
what_next handler_user_list ()/*{{{*/
{
	// List users

	require_club_admin ();

	// TODO extract the generic part of this list, like write_fields_display.
	document.write_paragraph (document.text_link (back_link_url (web_user_add), "Neu anlegen"));

	list<sk_user> users;
	int ret=db.sk_user_list (users);
	CHECK_DB_ERROR_ERROR;

	html_table table;

	// Write the table header
	table.push_back (make_table_header (fields_sk_user));

	// For each user, write a table row.
	list<sk_user>::const_iterator users_end=users.end ();
	for (list<sk_user>::const_iterator user=users.begin (); user!=users_end; ++user)
	{
		sk_user_to_fields (*user);
		html_table_row user_row=make_table_data_row (fields_sk_user);

		// Write the edit and delete cells
		user_row.push_back (make_link_cell (web_user_edit, "Editieren", arg_cgi_username, (*user).username));
		user_row.push_back (make_link_cell (web_user_delete, "L�schen", arg_cgi_username, (*user).username));

		// End of row
		table.push_back (user_row);
	}

	document.write (table);
	document.write_paragraph (num_to_string ((unsigned int)users.size ())+" Benutzer");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_user_delete ()/*{{{*/
{
	// Display user

	require_club_admin ();

	if (!CGI_HAS (username)) return what_next::output_error ("Kein Benutzername angegeben");
	string username=CGI_READ (username);
	if (username.empty ()) return what_next::output_error ("Benutzername ist leer");
	if (determine_user_class (username)!=uc_sk_user) return what_next::output_error ("Der Benutzername \""+username+"\" liegt nicht in der Benutzerklasse uc_sk_user");

	sk_user user;
	int ret=db.sk_user_get (user, username);
	CHECK_DB_ERROR_ERROR;
	sk_user_to_fields (user);

	require_not_self (user.username, "Man kann sich nicht selbst l�schen");
	require_matching_club_admin (user.club, "Es k�nnen nur Benutzer des eigenen Vereins gel�scht werden");

	document.write_paragraph ("Der folgende Benutzer wird gel�scht:");

	document.start_paragraph ();
	write_fields_display (fields_sk_user);
	document.end_paragraph ();

	document.start_paragraph ();
	write_delete_links (web_user_do_delete, arg_cgi_username, user.username, web_user_list, user.username);
	document.end_paragraph ();

	return what_next::output_document ();
}
/*}}}*/

what_next handler_user_do_delete ()/*{{{*/
{
	// Delete the user

	require_club_admin ();

	if (!CGI_HAS (username)) return what_next::output_error ("Kein Benutzername angegeben");
	string username=CGI_READ (username);
	if (username.empty ()) return what_next::output_error ("Benutzername ist leer");
	if (determine_user_class (username)!=uc_sk_user) return what_next::output_error ("Der Benutzername \""+username+"\" liegt nicht in der Benutzerklasse uc_sk_user");
	check_username (username);

	sk_user user_to_delete;
	int ret=db.sk_user_get (user_to_delete, username);
	CHECK_DB_ERROR_ERROR;

	require_not_self (username, "Man kann sich nicht selbst l�schen");
	require_matching_club_admin (user_to_delete.club, "Es k�nnen nur Benutzer des eigenen Vereins gel�scht werden");

	ret=db.sk_user_delete (username);
	CHECK_DB_ERROR_ERROR;

	return make_redirect (web_user_list);
}
/*}}}*/

what_next handler_user_add ()/*{{{*/
{
	// Permissions are checked by web_user_edit.
	cgi_args.set_value (arg_cgi_create_new);
	return what_next::go_to_state (web_user_edit).set_keep_title ();
}
/*}}}*/

what_next handler_user_edit ()/*{{{*/
{
	// TODO dass edit und add die gleiche Funktion ist, ist vielleicht doch
	// nicht so geschickt gewesen? Mal sehen, wie kurz man die Funktion unter
	// Verwendung von object_field bekommt.
	bool create_new=false;
	if (CGI_HAS (create_new)) create_new=true;
	if (session.args.has_argument (arg_session_create_new)) create_new=true;

	require_club_admin ();

	string username;
	if (!create_new)
	{
		if (CGI_HAS (username))
		{
			username=CGI_READ (username);
		}
		else
		{
			// If the username was not found in the CGI arguments, try the session.
			username=session.args.get_value (field_user_username);
		}

		if (username.empty ()) return what_next::output_error ("Kein Benutzername angegeben");
		check_username (username);
	}

	sk_user user;

	if (!create_new)
	{
		int ret=db.sk_user_get (user, username);
		CHECK_DB_ERROR_ERROR;

		require_matching_club_admin (user.club, "Man kann nur Benutzer des eigenen Vereins editieren");
		require_not_self (username, "Man kann sich nicht selbst editieren");
	}

	// When creating, this initializes the fields to the defaults.
	sk_user_to_fields (user);

	// If we are coming from web_person_select, we must enter the fields that
	// were given. These values are saved in the session.
	fields_read_from_session (fields_sk_user);

	// If we can freely choose the club, enter the person's club if empty so
	// far.
	// TODO: more abstraction: user_can_do_this instead of permission>=sk_admin
	if (create_new && provides (session_access, dba_sk_admin))
	{
		// Find the club and person fields
		list<object_field>::iterator fields_end=fields_sk_user.end ();
		list<object_field>::iterator field_club=fields_end;
		list<object_field>::iterator field_person=fields_end;
		for (list<object_field>::iterator it=fields_sk_user.begin (); it!=fields_end; ++it)
		{
			if ((*it).get_label ()==field_user_club) field_club=it;
			if ((*it).get_label ()==field_user_person) field_person=it;
		}

		// Check that the fields were found
		if (field_club==fields_end) return what_next::output_error ("Vereinsfeld in fields_sk_user nicht gefunden (Programmfehler)");
		if (field_person==fields_end) return what_next::output_error ("Personenfeld in fields_sk_user nicht gefunden (Programmfehler)");

		// If the club is empty...
		if (trim ((*field_club).get_string ()).empty ())
		{
			// ...and a person is given...
			db_id person_id=(*field_person).get_db_id ();
			if (!id_invalid (person_id))
			{
				// ...get it from the database...
				sk_person person;
				int ret=db.get_person (&person, person_id);
				CHECK_DB_ERROR_ERROR;

				// ...and enter its club.
				(*field_club).set_to (person.club);
			}
		}
	}

	// If we are coming from web_user_do_edit (error message), we must enter
	// the fields that were given. These values take precedence over those read
	// before.
	// If we are coming from web_person_select, the selection result is also
	// passed here.
	fields_from_cgi (fields_sk_user);


	// TODO Kopie von fields_sk_user anlegen oder so
	field_user_lock (fields_sk_user);

	fields_write_edit_form (fields_sk_user, relative_url, web_user_do_edit, create_new, arg_cgi_username, user.username);

	document.write_paragraph (document.text_link (back_link_url (web_user_list, user.username), "Zur Benutzerliste"));

	return what_next::output_document ();
}
/*}}}*/

what_next handler_user_do_edit ()/*{{{*/
{
	// Are we creating the user (as opposed to modifying it)?
	bool create_new=CGI_HAS (create_new);

	if (CGI_HAS (select_person))
	{
		// Display a person selector.
		// We first need to save the information entered so far in the session
		// so it can be read back later when returning to the editing page.

		// Read the fields from the CGI parameters.
		fields_from_cgi (fields_sk_user);

		// Save the fields to the session.
		fields_write_to_session (fields_sk_user);
		if (create_new) session.args.set_value (arg_session_create_new);

		// Now we can safely go to the selection state
		return what_next::go_to_state (web_person_select, "Person ausw�hlen");
	}

	require_club_admin ();

	int ret;
	string result_username;
	sk_user user;

	// Set the correct field lockage. Also sets the values for locked fields.
	field_user_lock (fields_sk_user);

	// Read the user from the CGI arguments
	fields_from_cgi (fields_sk_user);
	sk_user_from_fields (user);

	// Get the username to modify/add
	string username;
	if (create_new)
	{
		// Create
		username=user.username;
		if (!user_name_valid (username)) return what_next::go_to_state (web_user_edit, "Benutzername enth�lt ung�ltige Zeichen", true);

		// TODO: handle these checks via lock_fields
		if (user.perm_club_admin) require_sk_admin ("Nur der Startkladdenadministrator kann Vereinsadministratorrechte vergeben");
		if (user.perm_read_flight_db) require_sk_admin ("Nur der Startkladdenadministrator kann Datenbankleserechte vergeben");
	}
	else
	{
		// Edit
		username=CGI_READ (username);
		check_username (username);

		sk_user org_user;
		int ret=db.sk_user_get (org_user, username);
		CHECK_DB_ERROR_ERROR;

		// TODO handle these checks via lock_fields
		if (user.perm_club_admin!=org_user.perm_club_admin) require_sk_admin ("Nur der Startkladdenadministrator kann Vereinsadministratorrechte �ndern");
		if (user.perm_read_flight_db!=org_user.perm_read_flight_db) require_sk_admin ("Nur der Startkladdenadministrator kann Datenbankleserechte �ndern");
	}


	if (user.perm_club_admin && user.club.empty ())
		return what_next::go_to_state (web_user_edit, "F�r einen Vereinsadmin ist ein Verein n�tig", true);

	// Check the username
	if (username.empty ()) return what_next::go_to_state (web_user_edit, "Kein Benutzername angegeben", true);
	if (determine_user_class (username)!=uc_sk_user) return what_next::go_to_state (web_user_edit, "Der Benutzername \""+username+"\" liegt nicht in der Benutzerklasse uc_sk_user", true);

	if (!id_invalid (user.person))
	{
		sk_person person;
		ret=db.get_person (&person, user.person);
		if (ret==db_err_not_found) return what_next::go_to_state (web_user_edit, "Die Person "+num_to_string (user.person)+" existiert nicht", true);
		CHECK_DB_ERROR_ERROR;

		// TODO what about non-editable persons?
		string message;
		message+="Man kann nur Benutzer zu Personen des eigenen Vereins zuordnen (";
		if (session_user) message+="eigener Verein: "+session_user->club+", ";
		message+="Verein von "+person.text_name ()+": "+person.club;
		message+=")";

		if (!provides (session_access, dba_sk_admin))
			require_matching_club_admin (person.club, message, web_user_edit);
	}


	if (create_new)
	{
		// Create

		// Determine and check the passwords
		string password_1=CGI_READ_F (field_user_password);
		string password_2=CGI_READ_F (field_user_password_repeat);
		if (password_1.empty ()) return what_next::go_to_state (web_user_edit, "Kein Passwort angegeben", true);
		if (password_1!=password_2) return what_next::go_to_state (web_user_edit, "Passw�rter stimmen nicht �berein", true);

		// Check if the user exists
		ret=db.sk_user_exists (user.username);
		CHECK_DB_ERROR_ERROR;
		if (ret>0) return what_next::go_to_state (web_user_edit, "Der Benutzername \""+user.username+"\" existiert bereits", true);

		// Add the user
		ret=db.sk_user_add (user, password_1);
		CHECK_DB_ERROR_ERROR;
	}
	else
	{
		// Modify

		// Check permissions
		require_matching_club_admin (user.club, "Man kann nur Benutzer des eigenen Vereins editieren");
		require_not_self (username, "Man kann sich nicht selbst editieren");

		// Update the user.
		user.username=username;	// We don't allow user name changes for now.

		// Check if the user exists
		ret=db.sk_user_exists (user.username);
		CHECK_DB_ERROR_ERROR;
		if (ret==0) return what_next::output_error ("Der Benutzername \""+user.username+"\" existiert nicht. Query: "+db.get_last_query ());

		// Write the user to the database.
		ret=db.sk_user_modify (user);
		CHECK_DB_ERROR_ERROR;
	}

	return make_redirect (web_user_list, user.username);
}
/*}}}*/

what_next handler_user_change_password ()/*{{{*/
{
	int ret;
	require_club_admin ();

	// TODO this checking block occurs in multiple functions. Replace it by
	// more generic functions throwing the ex_write_error_document exception.
	// Then, make one function that does all these checks.
	if (!CGI_HAS (username)) return what_next::output_error ("Kein Benutzername angegeben");
	string username=CGI_READ (username);
	if (username.empty ()) return what_next::output_error ("Benutzername ist leer");
	if (determine_user_class (username)!=uc_sk_user) return what_next::output_error ("Der Benutzername \""+username+"\" liegt nicht in der Benutzerklasse uc_sk_user");
	check_username (username);

	document.write_paragraph ("Passwort�nderung f�r Benutzer "+username);

	// Get the user
	sk_user user;
	ret=db.sk_user_get (user, username);
	CHECK_DB_ERROR_ERROR;

	// Permission checking
	require_matching_club_admin (user.club, "Man kann nur Passw�rter von Benutzern des eigenen Vereins �ndern");

	html_table table;

	html_table_row row;
	row.push_back (html_table_cell::text ("Passwort:"));
	row.push_back (document.make_input_password (arg_cgi_new_password_1));
	table.push_back (row);

	row.clear ();
	row.push_back (html_table_cell::text ("Passwort wiederholen:"));
	row.push_back (document.make_input_password (arg_cgi_new_password_2));
	table.push_back (row);

	row.clear ();
	row.push_back (html_table_cell (document.make_submit ("�ndern"), false, 2));
	table.push_back (row);

	argument_list additional_args;
	additional_args.set_value (arg_cgi_username, username);

	document.start_tag ("form", "action=\""+relative_url+"\" method=\"POST\"");
	document.write (table);
	document.write (back_form_hidden (web_user_do_change_password));
	document.write_hidden_fields (additional_args);
	document.end_tag ("form");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_user_do_change_password ()/*{{{*/
{
	int ret;
	require_club_admin ();

	if (!CGI_HAS (username)) return what_next::output_error ("Kein Benutzername angegeben");
	if (!CGI_HAS (new_password_1)) return what_next::output_error ("Passwort nicht angegeben");
	if (!CGI_HAS (new_password_2)) return what_next::output_error ("Passwortwiederholung nicht angegeben");

	string username=CGI_READ (username);
	string new_password_1=CGI_READ (new_password_1);
	string new_password_2=CGI_READ (new_password_2);
	check_username (username);

	if (username.empty ()) return what_next::output_error ("Benutzername ist leer");
	if (determine_user_class (username)!=uc_sk_user) return what_next::output_error ("Der Benutzername \""+username+"\" liegt nicht in der Benutzerklasse uc_sk_user");

	// TODO add fuction for password checks
	if (new_password_1.empty ()) return what_next::go_to_state (web_user_change_password, "Passwort nicht angegeben", true);
	if (new_password_2!=new_password_1) return what_next::go_to_state (web_user_change_password, "Passw�rter stimmen nicht �berein", true);

	// Get the user
	sk_user user;
	ret=db.sk_user_get (user, username);
	CHECK_DB_ERROR_ERROR;

	// Permission checking
	require_matching_club_admin (user.club, "Man kann nur Passw�rter von Benutzern des eigenen Vereins �ndern");

	ret=db.sk_user_change_password (username, new_password_1);
	CHECK_DB_ERROR_STATE (web_user_change_password);

	// Success
	SESSION_WRITE (one_time_message, "Passwort�nderung erfolgt");

	// For linking back
	argument_list args;
	args.set_value (arg_cgi_username, username);
	return make_redirect (web_user_edit, args);
}
/*}}}*/

what_next handler_person_select ()/*{{{*/
{
	// TODO code duplication with handler_list_persons
	// TODO consider writing a redirect here to jump the the anchor of the
	// person currently selected

	// Read persons from the database
	QPtrList<sk_person> persons; persons.setAutoDelete (true);
	int ret=db.list_persons_all (persons);
	CHECK_DB_ERROR_ERROR;

	DO_SUB_ACTION (write_person_list (persons, true, web_user_edit, field_user_person, NULL, false, true));

	return what_next::output_document ();
}
/*}}}*/

what_next handler_master_data_import ()/*{{{*/
{
	// TODO allow sk_admin to upload, specifying the club.
	require_sk_user ();
	require_club_admin ();

	// TODO auslagern
	document.start_tag ("form", "action=\""+relative_url+"\" method=\"POST\" enctype=\""+http_document::mime_type_multipart_form_data+"\"");

	argument_list data_types;
	data_types.set_value (arg_cgi_data_type_person, "Personen");

	html_table table;
	html_table_row row;

	row.clear ();
	row.push_back (html_table_cell::text ("Datentyp"));
	row.push_back (html_table_cell (document.make_input_select (arg_cgi_data_type, data_types, CGI_READ (data_type))));
	table.push_back (row);

	row.clear ();
	row.push_back (html_table_cell::text ("Datei"));
	row.push_back (html_table_cell (document.make_input_file (arg_cgi_file)));
	table.push_back (row);

	row.clear ();
	row.push_back (html_table_cell (document.make_submit ("Absenden"), false, 2));
	table.push_back (row);

	document.write_paragraph ("Bitte den Datentyp und die Datei ausw�hlen. Die Daten werden dann eingelesen und �berpr�ft.");
	document.write (table);
	document.write (back_form_hidden (web_master_data_upload));

	document.end_tag ("form");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_master_data_upload ()/*{{{*/
{
	require_sk_user ();

	// Don't check club admin here, this state does not need database access.
	if (CGI_READ (data_type)!=arg_cgi_data_type_person)
		return what_next::output_error ("Ung�ltiger Datentyp");

	SESSION_WRITE (master_data_file, CGI_READ (file));
	SESSION_WRITE (master_data_filename, filenames.get_value (arg_cgi_file));

	argument_list additional_args;
	additional_args.set_value (arg_cgi_data_type, CGI_READ (data_type));
	return make_redirect (web_master_data_check, additional_args);
}
/*}}}*/

what_next handler_master_data_check ()/*{{{*/
{
	require_sk_user ();
	require_club_admin ();

	if (CGI_READ (data_type)!=arg_cgi_data_type_person)
		return what_next::output_error ("Ung�ltiger Datentyp");

	// If there is not data file, the page was probably reloaded after another
	// state was activated. Silently redirect to the form.
	if (!SESSION_HAS (master_data_file))
	{
		argument_list additional_args;
		additional_args.set_value (arg_cgi_data_type_person, CGI_READ (data_type));
		return make_redirect (web_master_data_import, additional_args);
	}

	string filename=SESSION_READ (master_data_filename);
	if (filename.empty ())
		// TODO besser zur�ck in den Auswahlzustand, aber Felder voreintragen
		return what_next::output_error ("Keine Datei angegeben");

	string csv=SESSION_READ (master_data_file);
	QPtrList<sk_person> persons; persons.setAutoDelete (true);


	// Make the person list from the CSV data
	DO_SUB_ACTION (csv_to_persons (csv, persons, session_user->club));

	if (persons.isEmpty ())
		return what_next::output_error ("Keine Personen in der CSV-Datei gefunden");

	// Check the persons for errors.
	list<sk_db::import_message> messages;
	db.import_check (persons, messages);
	db.import_identify (persons, messages);

	// Count the fatal errors
	unsigned int num_fatal_errors=0;
	list<sk_db::import_message>::const_iterator end=messages.end ();
	for (list<sk_db::import_message>::const_iterator it=messages.begin (); it!=end; ++it)
		if ((*it).fatal ()) ++num_fatal_errors;

	// If there are errors, display them.
	if (num_fatal_errors>0)
	{
		document.write_paragraph (html_escape ("Die Daten k�nnen nicht importiert werden, weil folgende Probleme bestehen:"), "error");

		DO_SUB_ACTION (write_message_list (messages));

		document
			.start_paragraph ()
			.write_text_link (back_link_url (web_list_persons), "Personenliste")
			.end_paragraph ()
			;

		return what_next::output_document ();
	}

	// There are no fatal errors. If there are any messages left, these are
	// displayed to the user in a separate table.
	if (!messages.empty ())
	{
		document.write_paragraph (html_escape ("Bitte folgende Personen speziell �berpr�fen:"));
		DO_SUB_ACTION (write_message_list (messages));
	}

	// Display the list of persons
	document.write_paragraph (html_escape ("Die folgenden Personen wurden aus der CSV-Datei \""+filename+"\" gelesen. Bitte �berpr�fen:"));
	DO_SUB_ACTION (write_person_list (persons, false, "", "", NULL, true));

	// Write a link to the next state for acepting or back to the selection state
	argument_list additional_args;
	additional_args.set_value (arg_cgi_data_type, CGI_READ (data_type));
	document
		.start_paragraph ()
		.write_text_link (back_link_url (web_master_data_do_import, additional_args), "OK").endl ()
		.write_text_link (back_link_url (web_master_data_import, additional_args), "Zur�ck").endl ()
		.end_paragraph ()
		;

	if (debug_enabled)
	{
		document.write_paragraph ("The file transferred was:");
		document.write_preformatted (SESSION_READ (master_data_file));
	}

	return what_next::output_document ();
}
/*}}}*/

what_next handler_master_data_do_import ()/*{{{*/
{
	require_sk_user ();
	require_club_admin ();

	if (!SESSION_HAS (master_data_file))
		return what_next::output_error ("Keine Daten vorhanden");

	if (CGI_READ (data_type)!=arg_cgi_data_type_person)
		return what_next::output_error ("Ung�ltiger Datentyp");

	string csv=SESSION_READ (master_data_file);
	QPtrList<sk_person> persons; persons.setAutoDelete (true);

	DO_SUB_ACTION (csv_to_persons (csv, persons, session_user->club));

	try
	{
		db.import_persons (persons);
	}
	catch (sk_db::import_message &m)
	{
		return what_next::output_error ("Problem beim Importieren: "+m.description (true));
	}
	catch (sk_exception &e)
	{
		return what_next::output_error (e.description (true));
	}

	SESSION_WRITE (one_time_message, num_to_string (persons.count ())+" Personen eingespielt");
	return make_redirect (web_list_persons);
}
/*}}}*/

what_next handler_person_logbook ()/*{{{*/
{
	require_sk_user ("Dem Benutzer "+session_username+" ist keine Person zugeordnet. Bitte als anderer Benutzer anmelden.");
	if (id_invalid (session_user->person))
		return what_next::output_error ("Dem Benutzer ist keine Person zugeordnet. Ein Vereinsadministrator oder der Startkladdenadministrator muss diese Zuordnung vornehmen.");

	sk_person person;
	int ret=db.get_person (&person, session_user->person);
	CHECK_DB_ERROR_ERROR;
	// TODO handle not found error explicitly

	document.write_paragraph ("Flugbuchabfrage f�r "+person.text_name ());

	document.start_tag ("form", "action=\""+relative_url+"\" method=\"GET\"");

	html_table table;
	html_table_row row;

	argument_list formats;
	formats.set_value (arg_cgi_format_html, "HTML");
	formats.set_value (arg_cgi_format_csv, "CSV");
	formats.set_value (arg_cgi_format_pdf, "PDF");
	formats.set_value (arg_cgi_format_latex, "LaTeX");
	add_select_input (table, "Format:", arg_cgi_format, formats, arg_cgi_format_html);

	add_date_inputs (table, true);

	// TODO: this list should be part of the logbook class
	argument_list fims;
	fims.set_value (arg_cgi_flight_instructor_mode_no, "Nein");
	fims.set_value (arg_cgi_flight_instructor_mode_strict, "Streng");
	fims.set_value (arg_cgi_flight_instructor_mode_loose, "Locker");
	add_select_input (table, "Fluglehrermodus:", arg_cgi_flight_instructor_mode, fims, arg_cgi_flight_instructor_mode_no);

	add_submit_input (table);

	// TODO make document.start_form or a form class
	document.write (table);
	document.write (back_form_hidden (web_do_person_logbook));
	document.end_tag ("form");

	document.write_hr ();

	list<string> fi_mode_explanation;
	// TODO: this list should be part of the logbook class
	fi_mode_explanation.push_back ("Nein: Nur Fl�ge als Pilot werden verwendet.");
	fi_mode_explanation.push_back ("Streng: Auch Fl�ge als Begleiter werden verwendet, wenn der Flugtyp \"Schulung\" ist.");
	fi_mode_explanation.push_back ("Locker: Alle Fl�ge als Begleiter werden verwendet.");

	document
		.start_paragraph ()
		.write_text ("Fluglehrermodus:")
		.write_text_list (fi_mode_explanation)
		.end_paragraph ()
		;


	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_person_logbook ()/*{{{*/
{
	// TODO This function is too large. Need more abstraction, for example a
	// counterpart to add_date_inputs.
	//
	// Make sure that a sk_user is logged in and a person is associated with
	// it.
	require_sk_user ();
	db_id person_id=session_user->person;
	if (id_invalid (person_id)) return what_next::output_error ("Dem Benutzer ist keine Person zugeordnet.");

	// Get the person.
	sk_person person;
	int ret=db.get_person (&person, person_id);
	CHECK_DB_ERROR_ERROR;

	// Read the output format and date specifications from the CGI arguments.
	string format=CGI_READ (format);
	string date_spec=CGI_READ (date_spec);
	string fim_text=CGI_READ (flight_instructor_mode);

	if (format.empty ()) return what_next::output_error ("Kein Format angegeben");
	if (date_spec.empty ()) return what_next::output_error ("Kein Datumsmodus angegeben");
	if (fim_text.empty ()) return what_next::output_error ("Kein Fluglehrermodus angegeben");

	// Determine the flight instructor mode/*{{{*/
	flugbuch_entry::flight_instructor_mode fim=flugbuch_entry::fim_no;
	if (fim_text==arg_cgi_flight_instructor_mode_no)
		fim=flugbuch_entry::fim_no;
	else if (fim_text==arg_cgi_flight_instructor_mode_strict)
		fim=flugbuch_entry::fim_strict;
	else if (fim_text==arg_cgi_flight_instructor_mode_loose)
		fim=flugbuch_entry::fim_loose;
	else
		return what_next::output_error ("Unbekannter Fluglehrermodus "+fim_text);
/*}}}*/

	flight_list flights; flights.setAutoDelete (true);
	string date_text;

	// Start the filename
	string filename="flugbuch_"+person.vorname+"-"+person.nachname+"_";

	// Step 1: read the flights we are interested in from the database./*{{{*/
	// TODO make sk_db frontend function for listing.
	// TODO remove QDate stuff
	if (date_spec==arg_cgi_date_spec_today)
	{
		// Generate the date.
		sk_date date=sk_date::current ();
		QDate q_date=(QDate)date;
		date_text=date.text ();
		// TODO append date_text and move down
		filename.append (date.text ());

		// List the flights.
		db.list_flights (flights, condition_t (cond_flight_person_date, person_id, &q_date));
	}
	else if (date_spec==arg_cgi_date_spec_single)
	{
		// Read and check the date.
		sk_date date=date_from_cgi (arg_cgi_date_single_year, arg_cgi_date_single_month, arg_cgi_date_single_day);
		if (date.is_invalid ()) return what_next::output_error ("Das angegebene Datum ist ung�ltig.");
		QDate q_date=(QDate)date;
		date_text=date.text ();
		filename.append (date.text ());

		// List the flights.
		db.list_flights (flights, condition_t (cond_flight_person_date, person_id, &q_date));
	}
	else if (date_spec==arg_cgi_date_spec_range)
	{
		// Read and check the start date.
		sk_date start_date=date_from_cgi (arg_cgi_date_start_year, arg_cgi_date_start_month, arg_cgi_date_start_day);
		if (start_date.is_invalid ()) return what_next::output_error ("Das angegebene Anfangsdatum ist ung�ltig.");
		QDate q_start_date=(QDate)start_date;

		// Read and check the end date.
		sk_date end_date=date_from_cgi (arg_cgi_date_end_year, arg_cgi_date_end_month, arg_cgi_date_end_day);
		if (end_date.is_invalid ()) return what_next::output_error ("Das angegebene Enddatum ist ung�ltig.");
		QDate q_end_date=(QDate)end_date;

		if (end_date<start_date) return what_next::output_error ("Das Enddatum liegt vor dem Anfangsdatum.");
		date_text=start_date.text ()+" bis "+end_date.text ();
		filename.append (start_date.text ()+"_"+end_date.text ());

		// List the flights.
		db.list_flights (flights, condition_t (cond_flight_person_date_range, person_id, &q_start_date, &q_end_date));
	}
	else
	{
		return what_next::output_error ("Unbekannter Datumsmodus \""+date_spec+"\"");
	}
/*}}}*/

	// Step 2: Make the personal logbook./*{{{*/
	QPtrList<flugbuch_entry> flugbuch; flugbuch.setAutoDelete (true);
	// We pass an empty QDate here because we already filtered for date above.
	make_flugbuch_person (flugbuch, &db, QDate (), &person, flights, fim);
	/*}}}*/

	// Step 3: Output the logbook./*{{{*/
	// We have got a list<flugbuch_entry>.
	if (format==arg_cgi_format_html)
	{
		document.write_paragraph ("Flugbuchabfrage f�r "+person.text_name ()+", "+date_text);

		if (flugbuch.isEmpty ())
		{
			document.write_paragraph ("Keine Fl�ge");
		}
		else
		{
			// Convert list<flugbuch_entry> to an html_table row by row.
			html_table table;

			// Write the table header
			table.push_back (make_table_header (fields_flugbuch_entry));

			// For each flugbuch entry, write a table row.
			for (QPtrListIterator<flugbuch_entry> it (flugbuch); *it; ++it)
			{
				flugbuch_entry_to_fields (**it);
				html_table_row user_row=make_table_data_row (fields_flugbuch_entry);

				// End of row
				table.push_back (user_row);
			}

			document.write (table);
			document.write_paragraph (num_to_string (flugbuch.count ())+" Eintr�ge");
		}

		return what_next::output_document ();
	}
	else if (format==arg_cgi_format_csv)
	{
		// Convert list<flugbuch_entry> to a table row by row.
		table tab;

		// Add the header
		tab.push_back (table_row_from_fields (fields_flugbuch_entry, true));

		// For each flugbuch entry, make a table row.
		for (QPtrListIterator<flugbuch_entry> it (flugbuch); *it; ++it)
		{
			flugbuch_entry_to_fields (**it, true);
			tab.push_back (table_row_from_fields (fields_flugbuch_entry));
		}

		filename.append (".csv");
		return what_next::output_raw_document (tab.csv (opts.csv_quote), http_document::mime_type_csv, filename, "Flugbuch f�r "+person.text_name ());
	}
	else if (format==arg_cgi_format_latex)
	{
		latex_document ldoc;
		write_flugbuch (ldoc, flugbuch, date_text, person.text_name ());

		filename.append (".tex");
		return what_next::output_raw_document (ldoc.get_string (), http_document::mime_type_plaintext, filename, "Flugbuch f�r "+person.text_name ());
	}
	else if (format==arg_cgi_format_pdf)
	{
		latex_document ldoc;
		write_flugbuch (ldoc, flugbuch, date_text, person.text_name ());

		filename.append (".pdf");
		try
		{
			string pdf=ldoc.make_pdf ();
			return what_next::output_raw_document (pdf, http_document::mime_type_pdf, filename, "Flugbuch f�r "+person.text_name ());
		}
		catch (latex_document::ex_command_failed &e)
		{
			return what_next::output_error (e.description ()+"\nBefehl: "+e.command+"\n\nAusgabe:\n"+e.output+"\n\nDokument:\n"+e.document);
		}
		catch (sk_exception &e)
		{
			return what_next::output_error (e.description ());
		}
	}
	else
	{
		return what_next::output_error ("\""+format+"\"? Komisches Format. Kenne ich nicht.");
	}
	/*}}}*/

	return what_next::output_error ("Unbehandelter Fall in handler_do_person_logbook ()");
}
/*}}}*/

what_next handler_plane_logbook ()/*{{{*/
{
	// Access without session is possible from local hosts. For other hosts, we
	// need a session. This requirement is checked in do_next.

	document.start_tag ("form", "action=\""+relative_url+"\" method=\"GET\"");

	html_table table;

	add_date_inputs (table, false);
	add_submit_input (table);

	document.write (table);
	document.write (back_form_hidden (web_do_plane_logbook));
	document.end_tag ("form");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_plane_logbook ()/*{{{*/
{
	// Access without session is possible from local hosts. For other hosts, we
	// need a session. This requirement is checked in do_next.

	flight_list flights;
	flights.setAutoDelete (true);
	QPtrList<sk_flugzeug> planes;
	planes.setAutoDelete (true);
	string date_text;

	string date_spec=CGI_READ (date_spec);
	if (date_spec.empty ())
		return what_next::output_error ("Kein Datumsmodus angegeben");

	// TODO!: more abstraction
	// Read the flights we are interested in from the database./*{{{*/
	// TODO remove QDate stuff
	// TODO code duplication with handler_do_flightlist
	sk_date date;
	if (date_spec==arg_cgi_date_spec_today)
	{
		// Generate the date.
		date=sk_date::current ();
	}
	else if (date_spec==arg_cgi_date_spec_single)
	{
		// Read and check the date.
		date=date_from_cgi (arg_cgi_date_single_year, arg_cgi_date_single_month, arg_cgi_date_single_day);
		if (date.is_invalid ())
			return what_next::output_error ("Das angegebene Datum ist ung�ltig.");
	}
	else if (date_spec==arg_cgi_date_spec_range)
	{
		return what_next::output_error ("Ung�ltiger Datumsmodus \""+date_spec+"\"");
	}
	else
	{
		return what_next::output_error ("Unbekannter Datumsmodus \""+date_spec+"\"");
	}

	// Make the date and the file name
	date_text=date.text ();
	string filename="bordbuch_"+date_text;

	// List the flights and planes
	// TODO remove QDate
	QDate q_date=(QDate)date;

	int ret=db.list_flights_date (flights, &q_date);
	CHECK_DB_ERROR_ERROR;
	flights.sort ();

	ret=db.list_planes_date (planes, &q_date);
	CHECK_DB_ERROR_ERROR;
///*}}}*/

	// Make the club list
	// TODO: this functionality could be moved to write_bordbuch
	list<string> club_list;
	make_unique_club_list (club_list, planes);

	// Make the logbook
	latex_document ldoc;
	ldoc.no_section_numbers=true;
	for (list<string>::const_iterator clubs_end=club_list.end (),
			club=club_list.begin (); club!=clubs_end; ++club)
	{
		string c=*club;

		// Generate the bordbuch
		QPtrList<bordbuch_entry> bordbuch; bordbuch.setAutoDelete (true);
			QPtrList<bordbuch_entry> bb;
		make_bordbuch_day (bordbuch, &db, q_date, planes, flights, &c);

		if (!bordbuch.isEmpty ())
		{
			if (eintrag_ist_leer (c))
				ldoc.start_section ("Kein Verein");
			else
				ldoc.start_section (c);

			// Output the logbook.
			write_bordbuch (ldoc, bordbuch, date_text);
			ldoc.write_empty_line ();
		}
	}

	// TODO!: more abstraction, and getting LaTeX here would be nice.
	filename.append (".pdf");
	try
	{
		string pdf=ldoc.make_pdf ();
		return what_next::output_raw_document (pdf, http_document::mime_type_pdf, filename, "Bordb�cher f�r "+date_text);
	}
	catch (latex_document::ex_command_failed &e)
	{
		return what_next::output_error (e.description ()+"\nBefehl: "+e.command+"\n\nAusgabe:\n"+e.output+"\n\nDokument:\n"+e.document);
	}
	catch (sk_exception &e)
	{
		return what_next::output_error (e.description ());
	}

	return what_next::output_error ("Unbehandelter Fall in handler_do_person_logbook ()");
}
/*}}}*/

what_next handler_flightlist ()/*{{{*/
{
	// Access without session is possible from local hosts. For other hosts, we
	// need a session. This requirement is checked in do_next.

	// TODO make document.start_form or a form class
	document.start_tag ("form", "action=\""+relative_url+"\" method=\"GET\"");

	html_table table;

	add_date_inputs (table, false);
	add_submit_input (table);

	document.write (table);
	document.write (back_form_hidden (web_do_flightlist));
	document.end_tag ("form");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_flightlist ()/*{{{*/
{
	// Access without session is possible from local hosts. For other hosts, we
	// need a session. This requirement is checked in do_next.

	flight_list flights; flights.setAutoDelete (true);
	string date_text;

	// TODO this pattern occurs quite often. Make function or something.
	string date_spec=CGI_READ (date_spec);
	if (date_spec.empty ()) return what_next::output_error ("Kein Datumsmodus angegeben");

	// Read the flights we are interested in from the database.
	// TODO remove QDate stuff
	sk_date date;
	if (date_spec==arg_cgi_date_spec_today)
	{
		// Generate the date.
		date=sk_date::current ();
	}
	else if (date_spec==arg_cgi_date_spec_single)
	{
		// Read and check the date.
		date=date_from_cgi (arg_cgi_date_single_year, arg_cgi_date_single_month, arg_cgi_date_single_day);
		if (date.is_invalid ()) return what_next::output_error ("Das angegebene Datum ist ung�ltig.");
	}
	else if (date_spec==arg_cgi_date_spec_range)
	{
		return what_next::output_error ("Ung�ltiger Datumsmodus \""+date_spec+"\"");
	}
	else
	{
		return what_next::output_error ("Unbekannter Datumsmodus \""+date_spec+"\"");
	}

	// Make the date and the file name
	date_text=date.text ();
	string filename="startliste_"+date_text;

	// List the flights.
	// TODO remove QDate
	QDate q_date=(QDate)date;
	int ret=db.list_flights_date (flights, &q_date);
	CHECK_DB_ERROR_ERROR;

	flights.sort ();

	// Output the list.
	latex_document ldoc;
	write_flightlist (ldoc, flights, date_text);

	filename.append (".pdf");
	try
	{
		string pdf=ldoc.make_pdf ();
		return what_next::output_raw_document (pdf, http_document::mime_type_pdf, filename, "Startliste f�r "+date_text);
	}
	catch (latex_document::ex_command_failed &e)
	{
		return what_next::output_error (e.description ()+"\nBefehl: "+e.command+"\n\nAusgabe:\n"+e.output+"\n\nDokument:\n"+e.document);
	}
	catch (sk_exception &e)
	{
		return what_next::output_error (e.description ());
	}
}
/*}}}*/

what_next handler_flight_db ()/*{{{*/
{
	require_read_flight_db ();

	// TODO make document.start_form or a form class
	document.start_tag ("form", "action=\""+relative_url+"\" method=\"GET\"");

	html_table table;
	html_table_row row;

	// TODO remove today and single, is not needed
	add_date_inputs (table, true);
	add_bool_input (table, "Schleppfl�ge einzeln", arg_cgi_towflights_extra);

	// Make a list of data formats available.
	argument_list data_formats;
	// The default data format is hardcoded and always available.
	data_formats.set_value (arg_cgi_data_format_default, "Standard");

	// Add the data formats provided by plugins to the list.
	list<plugin_data_format>::const_iterator plugins_end=opts.plugins_data_format.end ();
	for (list<plugin_data_format>::const_iterator plugin=opts.plugins_data_format.begin (); plugin!=plugins_end; ++plugin)
	{
		// If there is a problem in one of the plugins, go on trying the
		// others.
		try
		{
			argument_list this_plugin_formats=(*plugin).plugin_list_unique_formats ();

			argument_list::const_iterator formats_end=this_plugin_formats.end ();
			for (argument_list::const_iterator format=this_plugin_formats.begin (); format!=formats_end; ++format)
			{
				string label=(*format).get_name ();
				string caption=(*format).get_value ();
				if (debug_enabled)
					caption.append (" ("+(*plugin).get_real_filename ()+")");
				else
					caption.append (" (Plugin)");
				data_formats.set_value (label, caption);
			}
		}
		catch (plugin_data_format::exception &e)
		{
			debug_stream << "Fehler in Plugin " << (*plugin).get_display_filename () << ": "+e.description () << endl;
		}
	}

	if (data_formats.size ()>1)
		add_select_input (table, "Datenformat:", arg_cgi_data_format, data_formats, arg_cgi_data_format_default);

	add_submit_input (table);

	document.write (table);
	document.write (back_form_hidden (web_do_flight_db));
	document.end_tag ("form");

	return what_next::output_document ();
}
/*}}}*/

what_next handler_do_flight_db ()/*{{{*/
{
	require_read_flight_db ();

	// Read the output format and date specifications from the CGI arguments.
	// TODO code duplication with handler_do_person_logbook: date parsing,
	// flight reading, csv output
	string date_spec=CGI_READ (date_spec);
	if (date_spec.empty ()) return what_next::output_error ("Kein Datumsmodus angegeben");

	// Find out which data format to use
	bool use_default_data_format=true;
	string data_format;
	if (CGI_HAS (data_format))
	{
		data_format=CGI_READ (data_format);
		if (data_format!=arg_cgi_data_format_default)
			use_default_data_format=false;
	}

	flight_list flights;
	flights.setAutoDelete (true);
	string date_text;

	// Start the filename
	string filename="startkladde_";

	// Step 1: read the flights we are interested in from the database./*{{{*/
	// TODO remove QDate stuff
	if (date_spec==arg_cgi_date_spec_today)
	{
		// Generate the date.
		sk_date date=sk_date::current ();
		QDate q_date=(QDate)date;
		date_text=date.text ();
		filename.append (date.text ());

		// List the flights.
		db.list_flights_date (flights, &q_date);
	}
	else if (date_spec==arg_cgi_date_spec_single)
	{
		// Read and check the date.
		sk_date date=date_from_cgi (arg_cgi_date_single_year, arg_cgi_date_single_month, arg_cgi_date_single_day);
		if (date.is_invalid ()) return what_next::output_error ("Das angegebene Datum ist ung�ltig.");
		QDate q_date=(QDate)date;
		date_text=date.text ();
		filename.append (date.text ());

		// List the flights.
		db.list_flights_date (flights, &q_date);
	}
	else if (date_spec==arg_cgi_date_spec_range)
	{
		// Read and check the start date.
		sk_date start_date=date_from_cgi (arg_cgi_date_start_year, arg_cgi_date_start_month, arg_cgi_date_start_day);
		if (start_date.is_invalid ()) return what_next::output_error ("Das angegebene Anfangsdatum ist ung�ltig.");
		QDate q_start_date=(QDate)start_date;

		// Read and check the end date.
		sk_date end_date=date_from_cgi (arg_cgi_date_end_year, arg_cgi_date_end_month, arg_cgi_date_end_day);
		if (end_date.is_invalid ()) return what_next::output_error ("Das angegebene Enddatum ist ung�ltig.");
		QDate q_end_date=(QDate)end_date;

		if (end_date<start_date) return what_next::output_error ("Das Enddatum liegt vor dem Anfangsdatum.");
		date_text=start_date.text ()+" bis "+end_date.text ();
		filename.append (start_date.text ()+"_"+end_date.text ());

		// List the flights.
		db.list_flights_date_range (flights, &q_start_date, &q_end_date);
	}
	else
	{
		return what_next::output_error ("Unbekannter Datumsmodus \""+date_spec+"\"");
	}
/*}}}*/

	// Step 2: Make the flight list./*{{{*/
	// If the towflights should have own entries, add them.
	if (string_to_bool (CGI_READ (towflights_extra)))
	{
		flight_list towflights;	// No autoDelete because we add them to flights later.

		for (QPtrListIterator<sk_flug> it (flights); *it; ++it)
		{
			int ret;

			// Read the startart from the database
			startart_t sa;
			ret=db.get_startart (&sa, (*it)->startart);
			bool sa_ok=(ret==db_ok);

			// If the startart is an airtow, we need to construct the towflight.
			if (sa_ok && sa.is_airtow ())
			{
				// Determine the ID of the towplane.
				db_id towplane_id;
				if (sa.towplane_known ())
				{
					// The towplane is known by registration. Get its ID.
					sk_flugzeug sfz;
					ret=db.get_plane_registration (&sfz, sa.get_towplane ());
					if (ret==db_ok)
						towplane_id=sfz.id;
					else
						towplane_id=invalid_id;
				}
				else
				{
					// The towplane is not known from the startart, so its ID
					// is saved in the flight.
					towplane_id=(*it)->towplane;
				}

				// Determine the startart.
				db_id towflight_startart_id;
				startart_t ss;
				if (db.get_startart_by_type (&ss, sat_self)==db_ok)
					towflight_startart_id=ss.get_id ();
				else
					towflight_startart_id=invalid_id;

				sk_flug *towflight=new sk_flug;

				(*it)->get_towflight (towflight, towplane_id, towflight_startart_id);
				towflights.append (towflight);
			}

		}

		// Copy the towflights to the flight list
		for (QPtrListIterator<sk_flug> tow (towflights); *tow; ++tow)
		{
			flights.append (*tow);
		}
	}
	flights.sort ();
	/*}}}*/

	// Step 3: Output the list./*{{{*/
	// We have got a flight_list
	// Convert flight_list to a table row by row.
	table tab;

	// If we do not use the default data format, find the plugin that provides
	// the data format to use. Output an error if it is not found.
	const plugin_data_format *format_plugin=NULL;
	if (!use_default_data_format)
	{
		if (data_format!=arg_cgi_data_format_default)
		{
			for (list<plugin_data_format>::const_iterator plugins_end=opts.plugins_data_format.end (),
				plugin=opts.plugins_data_format.begin (); plugin!=plugins_end; ++plugin)
			{
				// If there is a problem with this plugin, go on trying the
				// other plugins.
				try
				{
					if ((*plugin).provides_unique_format (data_format))
					{
						format_plugin=&*plugin;
						break;
					}
				}
				catch (plugin_data_format::exception &e)
				{
					debug_stream << "Fehler in Plugin " << (*plugin).get_display_filename () << ": "+e.description () << endl;
				}
			}
		}

		if (!format_plugin)
			return what_next::output_error ("Datenformat \""+data_format+"\" nicht gefunden.");
	}

	// If there is a problem with this plugin, there is no point in continuing.
	try
	{
		// Determine the field list.
		list<object_field> fields;
		if (use_default_data_format)
			fields=fields_flight_db_entry;
		else
			format_plugin->plugin_make_field_list (data_format, fields);

		// Add the header
		tab.push_back (table_row_from_fields (fields, true));

		// For each list entry, make a table row.
		QDate old_date;
		int num;

		for (QPtrListIterator<sk_flug> it (flights); *it; ++it)
		{
			sk_flug &f=**it;

			QDate this_date=(*it)->effdatum ();
			if (old_date.isNull () || this_date!=old_date)
				num=1;
			else
				num++;
			old_date=this_date;

			sk_flug_data flight_data=sk_flug_data::owner ();
			db.make_flight_data (flight_data, f);

			try
			{
				if (use_default_data_format)
					flight_to_fields (fields, f, flight_data, num);
				else
					format_plugin->plugin_flight_to_fields (data_format, fields, f, flight_data, num, "", "???");
			}
			catch (plugin_data_format::ex_plugin_internal_error &e)
			{
				// If the error is not fatal, we can continue. Else, we throw
				// it again to cancel output.
				if (e.fatal) throw e;
			}

			tab.push_back (table_row_from_fields (fields));
		}
	}
	catch (plugin_data_format::exception &e)
	{
		return what_next::output_error ("Fehler in Plugin "+format_plugin->get_display_filename ()+": "+e.description ());
	}

	filename.append (".csv");
	return what_next::output_raw_document (tab.csv (opts.csv_quote), http_document::mime_type_csv, filename, "Flugliste f�r "+date_text);
	/*}}}*/

	return what_next::output_error ("Unbehandelter Fall in handler_do_flight_db");
}
/*}}}*/

what_next handler_test_redirect ()/*{{{*/
{
	string url;
	if (CGI_HAS (url))
		return what_next::do_redirect (CGI_READ (url));
	else
		return make_redirect (web_main_menu);
}
///*}}}*/


//what_next handler_ ()/*{{{*/
//{
//	return what_next::output_error ("\""+current_state->make_caption ()+"\" ist nicht implementiert.");	// TODO
//}
///*}}}*/





void cleanup_session (const string &state)/*{{{*/
{
	// Session variables are not cleaned up immediately in order to be able to
	// handle reloads.
	if (state!=web_result) SESSION_REMOVE (result_text);
	if (state!=web_result) SESSION_REMOVE (result_error);
	if (state!=web_result) SESSION_REMOVE (result_state);

	if (state!=web_person_select && state!=web_user_edit) fields_delete_from_session (fields_sk_user);

	if (state!=web_master_data_check && state!=web_master_data_do_import) SESSION_REMOVE (master_data_file);
	if (state!=web_master_data_check && state!=web_master_data_do_import) SESSION_REMOVE (master_data_filename);
}
/*}}}*/

what_next do_next (const what_next next, http_document &http)/*{{{*/
	// TODO generic? this should best be a member of what_next
{
	try
	{
		switch (next.get_next ())
		{
			case wn_go_on:/*{{{*/
			{
				// Go to the output_error state, because wn_go_on may not be used
				// as a program state.
				return what_next::output_error ("Zustand wn_go_on ausgef�hrt (Programmfehler)");
			} break;/*}}}*/
			case wn_output_error:/*{{{*/
			{
				// Output an error document.
				// That means: clear the current document (this may be called after
				// parts of the document have been output), and write the error
				// message. Then, change to the output_document state.
				document.clear ();
				// TODO add state.make_caption. Problem is, we don't know about
				// state here.
				document.set_title (caption_prefix+"Fehler");
				document.write_error_paragraph (html_escape (next.get_message ()));
				if (!next.get_explanation ().empty ()) document.write_paragraph (next.get_explanation ());
				if (session.is_ok ())
					document.write_paragraph (document.text_link (back_link_url (web_main_menu), "Hauptmen�"));
				else
					document.write_paragraph (document.text_link (back_link_url (web_logout), "Anmeldeseite"));
				return what_next::output_document ();
			} break;/*}}}*/
			case wn_output_document:/*{{{*/
			{
				// Output the document.
				// That means: write the document footer. Write the current
				// html document to stdout with http headers. Then, change
				// to the end_program state.
				write_document_footer ();

				http.output (document);
				return what_next::end_program ();
			} break;/*}}}*/
			case wn_output_raw_document:/*{{{*/
			{
				// Output a raw document.
				// That means: write the raw document to stdout with http headers.
				// Then, change to the end_program state.

				http.content_disposition="attachment";	// TODO symb. const. in http_document
				http.content_filename=next.get_filename ();
				http.content_description=next.get_description ();
				http.output (next.get_document (), next.get_mime_type ());
				return what_next::end_program ();
			} break;/*}}}*/
			case wn_change_state:/*{{{*/
			{
				// Go to a different state.
				// That means: clear the current document if the new state has
				// output (this may be called after parts of the document have been
				// output), set the new document title, write the message if one
				// exists, do some more setup. Then, the handler for the new state
				// is called, if it exists. If it doesn't, change to the
				// output_error state.  We return whatever the handler returns.

				// Get the new state, as what_next only saves the id.
				const web_interface_state *p_state;
				try
				{
					p_state=&(web_interface_state::from_list (next.get_state_label ()));
				}
				catch (web_interface_state::ex_not_found)
				{
					return what_next::output_error ("Ung�ltiger Zustand "+next.get_state_label ());
				}
				const web_interface_state &state=*p_state;

				cleanup_session (state.get_label ());

				// Clear the document unconditionally, even if the state
				// has no output, because what we might already have
				// written ist quite probably wrong as is is from a
				// different state (or at least a different incarnation of
				// the same state).
				string old_title=document.get_title ();
				document.clear ();
				if (next.get_keep_title ())
					document.set_title (old_title);
				else
					document.set_title (caption_prefix+state.make_caption ());

				// If a message is set, display it before the main document body.
				if (!next.get_message ().empty ())
				{
					if (next.get_message_error ())
						document.write_error_paragraph (next.get_message ());
					else
						document.write_paragraph (next.get_message ());
				}

				// Some state have certain preconditions, for example, that a
				// session is needed or that access without a session is only
				// allowed for local hosts. If these preconditions are not met,
				// this is an error.
				// The case of a given, but invalid session ID is handled in
				// setup_variables ().

				bool access_allowed=false;

				if (state.get_allow_anon ())
					// We can even use this state anonymously. Go on.
					access_allowed=true;
				else if (session.is_ok ())
					// We have a session, so we can use all state. Go on.
					// Further authorization may be done by the state handlers.
					access_allowed=true;
				else if (state.get_allow_local () && client_is_local)
					// We can use this state anoymously from local hosts *and* the
					// connection is from a local host. Go on.
					// Further authorization may be done by the state handlers.
					access_allowed=true;
				else if (request_method==rm_commandline)
					// We have command line access, so we have all permissions.
					// This is justified by the fact that every user that can
					// call this program from the command line (and has
					// permissions to read the password from the config file,
					// without which the database won't let us in anyway) could
					// also access the database directly.
					access_allowed=true;

				if (!access_allowed)
				{
					string error_message=string ("Zugriff nicht gestattet (")
						+"state=\""+state.get_label ()+"\"; "
						+"session_ok="+bool_to_string (session.is_ok ())+"; "
						+"client_is_local="+bool_to_string (client_is_local)+" ("+remote_address+")"
						+")";
					string error_description;

					if (state.get_allow_local ())
						error_description="Diese Aktion kann nur von lokalen Rechnern oder nach erfolgter Anmeldung durchgef�hrt werden.";
					else
						error_description="Diese Aktion kann nur nach erfolgter Anmeldung durchgef�hrt werden.";

					return what_next::output_error (error_message, error_description);
				}

				// If we don't have enough db access level, this is an error.
				// The access level present has been determined from the login data
				// in the session.
				db_access_t db_access_needed=state.get_db_access_needed ();
				if (!provides (session_access, db_access_needed))
				{
					// The access level present does not provide the access level
					// needed. This is a problem.

					return what_next::output_error (
							"Unzureichender Datenbankzugriff"
							" (vorhanden: "+db_access_string (session_access)+";"
							" ben�tigt: "+db_access_string (db_access_needed)+")");
				}

				// If a database connection is required, make sure it is
				// established. The user name and password has already been set in
				// setup_variables.
				if (db_access_needed!=dba_none && !db.connected ())
				{
					try
					{
						db.connect ();
						db.use_db ();

						try
						{
							db.check_usability ();
						}
						catch (sk_exception &e)
						{
							return what_next::output_error ("Datenbank ist nicht benutzbar. Grund: "+e.description ());
						}

						// Now that we have the database, we can set session_user, if appropriate.
						// TODO code duplication with authenticate
						if (session_user) delete session_user; session_user=NULL;
						if (session_access==dba_sk_user)
						{
							sk_user *_session_user;
							_session_user=new sk_user;
							int ret=db.sk_user_get (*_session_user, session_username);
							if (ret!=db_ok) return what_next::output_error ("Fehler beim Lesen des Benutzers \""+session_username+"\": "+db.db_error_description (ret, true));
							session_user=_session_user;
						}
					}
					catch (sk_exception &e)
					{
						return what_next::output_error ("Fehler beim Wiederherstellen der Datenbankverbindung: "+e.description ());
					}
				}

				if (SESSION_HAS (one_time_message))
					document.write_paragraph (html_escape (SESSION_READ (one_time_message)));
				SESSION_REMOVE (one_time_message);

				state_handler handler=state.get_handler ();
				if (!handler) return what_next::output_error ("Handler f�r Zustand \""+state.get_label ()+"\" nicht gesetzt (Programmfehler)");
				current_state=p_state;
				try
				{
					return handler ();
				}
				catch (ex_write_error_document &e)
				{
					string message=e.message;
					if (e.program_error) message+=" (Programmfehler)";
					return what_next::output_error (message);
				}
				catch (ex_go_to_state &e)
				{
					return what_next::go_to_state (e.state, e.message, e.error);
				}
			} break;/*}}}*/
			case wn_do_redirect:/*{{{*/
			{
				if (CGI_HAS (no_redirects))
				{
					document.clear ();
					document
						.write_paragraph (
								html_escape ("Suppressed redirect to ")+
								document.text_link (next.get_url (), next.get_url ())
								)
						;
					return what_next::output_document ();
				}
				else
				{
					http.output_redirect (next.get_url ());
					return what_next::end_program ();
				}
			} break;/*}}}*/
			case wn_end_program:/*{{{*/
			{
				// Go to the output_error state, because this function should not
				// have been called when wn_end_program was commanded.
				return what_next::output_error ("Zustand wn_end_program ausgef�hrt (Programmfehler)");
			} break;/*}}}*/
		}
	}
	catch (std::exception &e)
	{
		// An unhandled exception. This is very, very bad.
		return what_next::output_error ("Unbehandelte exception: "+string (e.what ()));
	}
	catch (...)
	{
		// An unhandled exception. This is very, very bad.
		return what_next::output_error ("Unbehandelte exception. Gro�er, b�ser Programmfehler. So kann ich nicht arbeiten.");
	}

	// It may happen that one of the what_next_t cases has been handled above,
	// but it did not return anything as it should. This is caught here.
	return what_next::output_error ("Unbehandelter Zustand in do_next () (Programmfehler)");
}
/*}}}*/

void setup_static_data ()/*{{{*/
{
	// caption bei states ohne Ausgabe: Zum Beispiel f�r error-document verwendet

#define ADD_HANDLER(STATE, HAS_OUTPUT, DB_ACCESS_NEEDED, ALLOW_ANON, ALLOW_LOCAL, CAPTION) \
	do	\
	{	\
	web_interface_state::add_to_list (web_interface_state (web_ ## STATE, HAS_OUTPUT, DB_ACCESS_NEEDED, ALLOW_ANON, ALLOW_LOCAL, CAPTION, handler_ ## STATE));	\
	} while (false)

	// _STATE_
	// anon local meaning
	//    0     0 logged-in only
	//    0     1 free access from local
	//    1     0 anonymous
	//    1     1 anonymous
	//                                                             allow_______
	//           state                        output  db needed    anon   local    caption
	ADD_HANDLER (login                      , true  , dba_none,    true , false,   "Anmelden"                  );
	ADD_HANDLER (do_login                   , false , dba_access,  true , false,   "Anmelden"                  );
	ADD_HANDLER (main_menu                  , true  , dba_none,    true , true,    "Hauptmen�"                 );
	ADD_HANDLER (logout                     , false , dba_none,    true , false,   "Abmelden"                  );
	ADD_HANDLER (change_password            , true  , dba_none,    false, false,   "Passwort �ndern"           );
	ADD_HANDLER (do_change_password         , false , dba_sk_user, false, false,   "Passwort �ndern"           );
	ADD_HANDLER (list_persons               , true  , dba_sk_user, false, false,   "Personen auflisten"        );
	ADD_HANDLER (display_person             , true  , dba_sk_user, false, false,   "Person anzeigen"           );
	ADD_HANDLER (edit_person                , true  , dba_sk_user, false, false,   "Person editieren"          );
	ADD_HANDLER (do_edit_person             , false , dba_sk_user, false, false,   "Person editieren"          );
	ADD_HANDLER (result                     , true  , dba_none,    true , false,   "Ergebnis"                  );
	ADD_HANDLER (delete_person              , true  , dba_sk_user, false, false,   "Person l�schen"            );
	ADD_HANDLER (do_delete_person           , false , dba_sk_user, false, false,   "Person l�schen"            );
	ADD_HANDLER (create_person              , true  , dba_sk_user, false, false,   "Person anlegen"            );
	ADD_HANDLER (do_create_person           , true  , dba_sk_user, false, false,   "Person anlegen"            );
	ADD_HANDLER (select_merge_person        , true  , dba_sk_user, false, false,   "Person �berschreiben"      );
	ADD_HANDLER (merge_person               , true  , dba_sk_user, false, false,   "Person �berschreiben"      );
	ADD_HANDLER (do_merge_person            , false , dba_sk_user, false, false,   "Person �berschreiben"      );

	ADD_HANDLER (user_list                  , true  , dba_sk_user, false, false,   "Benutzerliste"             );
	ADD_HANDLER (user_delete                , true  , dba_sk_user, false, false,   "Benutzer l�schen"          );
	ADD_HANDLER (user_do_delete             , false , dba_sk_user, false, false,   "Benutzer l�schen"          );
	ADD_HANDLER (user_add                   , false , dba_none,    false, false,   "Benutzer anlegen"          );
	ADD_HANDLER (user_edit                  , true  , dba_sk_user, false, false,   "Benutzer editieren"        );
	ADD_HANDLER (user_do_edit               , false , dba_sk_user, false, false,   "Benutzer editieren"        );
	ADD_HANDLER (user_change_password       , true  , dba_sk_user, false, false,   "Benutzerpasswort �ndern"   );
	ADD_HANDLER (user_do_change_password    , false , dba_sk_user, false, false,   "Benutzerpasswort �ndern"   );
	ADD_HANDLER (person_select              , true  , dba_sk_user, false, false,   "Person ausw�hlen"          );

	ADD_HANDLER (master_data_import         , true  , dba_sk_user, false, false,   "Stammdaten einspielen"     );
	ADD_HANDLER (master_data_upload         , false , dba_none,    false, false,   "Stammdaten einspielen"     );
	ADD_HANDLER (master_data_check          , true  , dba_sk_user, false, false,   "Stammdaten einspielen"     );
	ADD_HANDLER (master_data_do_import      , false , dba_sk_user, false, false,   "Stammdaten einspielen"     );

	ADD_HANDLER (person_logbook             , true  , dba_sk_user, false, false,   "Flugbuch abrufen"          );
	ADD_HANDLER (do_person_logbook          , true  , dba_sk_user, false, false,   "Flugbuch abrufen"          );
	ADD_HANDLER (plane_logbook              , true  , dba_access , false, true ,   "Bordb�cher abrufen"        );
	ADD_HANDLER (do_plane_logbook           , true  , dba_access , false, true ,   "Bordb�cher abrufen"        );
    ADD_HANDLER (flightlist                 , true  , dba_access , false, true ,   "Startliste abrufen"        );
    ADD_HANDLER (do_flightlist              , true  , dba_access , false, true ,   "Startliste abrufen"        );
    ADD_HANDLER (flight_db                  , true  , dba_sk_user, false, false,   "Flugdatenbank abrufen"     );
    ADD_HANDLER (do_flight_db               , true  , dba_sk_user, false, false,   "Flugdatenbank abrufen"     );

    ADD_HANDLER (test_redirect              , false , dba_none,    true,  true ,   "Redirects testen"          );
#undef ADD_HANDLER

#define OF object_field
#define OOF object_field::output_field
#define WEB_CHPW web_user_change_password
#define CGI_SEL arg_cgi_select_person
	//                                                                     list___ edit____________________________      create__________________________
	//                            caption                 data_type        display display edit   state     caption      display edit   state     caption      label
	fields_sk_user.push_back (OF ("Benutzername",         OF::dt_string,   true ,  true ,  false, "",       "",          false,  true,  "",       "",          field_user_username        ));
	fields_sk_user.push_back (OF ("Person",               OF::dt_db_id,    true ,  false,  true,  CGI_SEL,  "Ausw�hlen", false,  true,  CGI_SEL,  "Ausw�hlen", field_user_person          ));
	fields_sk_user.push_back (OF ("Passwort",             OF::dt_password, false,  false,  false, WEB_CHPW, "�ndern",    false,  true,  "",       "",          field_user_password        ));
	fields_sk_user.push_back (OF ("Passwort wiederholen", OF::dt_password, false,  false,  false, "",       "",          false,  true,  "",       "",          field_user_password_repeat ));
	fields_sk_user.push_back (OF ("Vereinsadmin",         OF::dt_bool,     true ,  false,  true , "",       "",          false,  true,  "",       "",          field_user_club_admin      ));
	fields_sk_user.push_back (OF ("Flugdatenbank lesen",  OF::dt_bool,     true ,  false,  true , "",       "",          false,  true,  "",       "",          field_user_read_flight_db  ));
	fields_sk_user.push_back (OF ("Verein",               OF::dt_string,   true ,  false,  true , "",       "",          false,  true,  "",       "",          field_user_club            ));

	// Output only

	//                                    caption            label
	fields_flugbuch_entry.push_back (OOF ("Tag"            , field_flugbuch_tag             ).set_no_break ()); widths_flugbuch_entry.push_back (15);	// "Tag"
	fields_flugbuch_entry.push_back (OOF ("Muster"         , field_flugbuch_muster          ));                 widths_flugbuch_entry.push_back (12);	// "Muster"
	fields_flugbuch_entry.push_back (OOF ("Kennzeichen"    , field_flugbuch_kennzeichen     ));                 widths_flugbuch_entry.push_back (14);	// "Kennzeichen"
	fields_flugbuch_entry.push_back (OOF ("Flugzeugf�hrer" , field_flugbuch_flugzeugfuehrer ));                 widths_flugbuch_entry.push_back (20);	// "Flugzeugf�hrer"
	fields_flugbuch_entry.push_back (OOF ("Begleiter"      , field_flugbuch_begleiter       ));                 widths_flugbuch_entry.push_back (20);	// "Begleiter"
	fields_flugbuch_entry.push_back (OOF ("Startart"       , field_flugbuch_startart        ));                 widths_flugbuch_entry.push_back (10);	// "Startart"
	fields_flugbuch_entry.push_back (OOF ("Ort Start"      , field_flugbuch_ort_start       ));                 widths_flugbuch_entry.push_back (15);	// "Ort Start"
	fields_flugbuch_entry.push_back (OOF ("Ort Landung"    , field_flugbuch_ort_landung     ));                 widths_flugbuch_entry.push_back (15);	// "Ort Landung"
	fields_flugbuch_entry.push_back (OOF ("Zeit Start"     , field_flugbuch_zeit_start      ));                 widths_flugbuch_entry.push_back (13);	// "Zeit Start"
	fields_flugbuch_entry.push_back (OOF ("Zeit Landung"   , field_flugbuch_zeit_landung    ));                 widths_flugbuch_entry.push_back (13);	// "Zeit Landung"
	fields_flugbuch_entry.push_back (OOF ("Flugdauer"      , field_flugbuch_flugdauer       ));                 widths_flugbuch_entry.push_back (13);	// "Flugdauer"
	fields_flugbuch_entry.push_back (OOF ("Bemerkung"      , field_flugbuch_bemerkung       ));                 widths_flugbuch_entry.push_back (20);	// "Bemerkung"

	//                                    caption            label
	fields_bordbuch_entry.push_back (OOF ("Datum"          , field_bordbuch_date         ).set_no_break ()); widths_bordbuch_entry.push_back (15);	// "Datum"
	fields_bordbuch_entry.push_back (OOF ("Verein"         , field_bordbuch_club         ));                 widths_bordbuch_entry.push_back (25);	// "Verein"
	fields_bordbuch_entry.push_back (OOF ("Kennz."         , field_bordbuch_registration ));                 widths_bordbuch_entry.push_back (14);	// "Kennz."
	fields_bordbuch_entry.push_back (OOF ("Typ"            , field_bordbuch_plane_type   ));                 widths_bordbuch_entry.push_back (16);	// "Typ"
	fields_bordbuch_entry.push_back (OOF ("Name"           , field_bordbuch_name         ));                 widths_bordbuch_entry.push_back (27);	// "Name"
	fields_bordbuch_entry.push_back (OOF ("Ins."           , field_bordbuch_num_persons  ));                 widths_bordbuch_entry.push_back (06);	// "Ins."
	fields_bordbuch_entry.push_back (OOF ("Startort"       , field_bordbuch_place_from   ));                 widths_bordbuch_entry.push_back (19);	// "Startort"
	fields_bordbuch_entry.push_back (OOF ("Zielort"        , field_bordbuch_place_to     ));                 widths_bordbuch_entry.push_back (19);	// "Zielort"
	fields_bordbuch_entry.push_back (OOF ("Start"          , field_bordbuch_starttime    ));                 widths_bordbuch_entry.push_back ( 9);	// "Start"
	fields_bordbuch_entry.push_back (OOF ("Landg."         , field_bordbuch_landtime     ));                 widths_bordbuch_entry.push_back ( 9);	// "Landg."
	fields_bordbuch_entry.push_back (OOF ("#Ldg."          , field_bordbuch_num_landings ));                 widths_bordbuch_entry.push_back ( 9);	// "#Ldg."
	fields_bordbuch_entry.push_back (OOF ("Dauer"          , field_bordbuch_flight_time  ));                 widths_bordbuch_entry.push_back ( 8);	// "Dauer"

	//                                      caption         label
	fields_flightlist_entry.push_back (OOF ("Nr."         , field_flight_number            )); widths_flightlist.push_back ( 5);	// Nr.
	fields_flightlist_entry.push_back (OOF ("Kennz."      , field_flight_registration      )); widths_flightlist.push_back (16);	// Kennz.
	fields_flightlist_entry.push_back (OOF ("Typ"         , field_flight_type              )); widths_flightlist.push_back (20);	// Typ
	fields_flightlist_entry.push_back (OOF ("Pilot"       , field_flight_pilot             )); widths_flightlist.push_back (27);	// Pilot
	fields_flightlist_entry.push_back (OOF ("Begleiter"   , field_flight_copilot           )); widths_flightlist.push_back (27);	// Begleiter
	fields_flightlist_entry.push_back (OOF ("Verein"      , field_flight_club              )); widths_flightlist.push_back (25);	// Verein
	fields_flightlist_entry.push_back (OOF ("SA"          , field_flight_startart          )); widths_flightlist.push_back ( 6);	// SA
	fields_flightlist_entry.push_back (OOF ("Start"       , field_flight_starttime         )); widths_flightlist.push_back ( 9);	// Start
	fields_flightlist_entry.push_back (OOF ("Landg."      , field_flight_landtime          )); widths_flightlist.push_back ( 9);	// Landg.
	fields_flightlist_entry.push_back (OOF ("Dauer"       , field_flight_duration          )); widths_flightlist.push_back ( 9);	// Dauer
	fields_flightlist_entry.push_back (OOF ("Ld. Sfz."    , field_flight_landtime_towplane )); widths_flightlist.push_back (11);	// Ld. Sfz.
	fields_flightlist_entry.push_back (OOF ("Dauer"       , field_flight_duration_towplane )); widths_flightlist.push_back ( 9);	// Dauer
	fields_flightlist_entry.push_back (OOF ("#Ldg."       , field_flight_num_landings      )); widths_flightlist.push_back ( 9);	// \\#Ldg.
	fields_flightlist_entry.push_back (OOF ("Startort"    , field_flight_startort          )); widths_flightlist.push_back (19);	// Startort
	fields_flightlist_entry.push_back (OOF ("Zielort"     , field_flight_zielort           )); widths_flightlist.push_back (19);	// Zielort
	fields_flightlist_entry.push_back (OOF ("Zielort SFZ" , field_flight_zielort_towplane  )); widths_flightlist.push_back (19);	// Zielort SFZ
	fields_flightlist_entry.push_back (OOF ("Bemerkung"   , field_flight_comments          )); widths_flightlist.push_back (22);	// Bemerkung

	fields_flight_db_entry.push_back (OOF ("Datum"                       , field_flight_date                  ));	// Datum
	fields_flight_db_entry.push_back (OOF ("Nummer"                      , field_flight_number                ));	// Nummer
	fields_flight_db_entry.push_back (OOF ("Kennzeichen"                 , field_flight_registration          ));	// Kennzeichen
	fields_flight_db_entry.push_back (OOF ("Typ"                         , field_flight_type                  ));	// Typ
	fields_flight_db_entry.push_back (OOF ("Flugzeug Verein"             , field_flight_plane_club            ));	// Flugzeug Verein
	fields_flight_db_entry.push_back (OOF ("Pilot Nachname"              , field_flight_pilot_last_name       ));	// Pilot Nachname
	fields_flight_db_entry.push_back (OOF ("Pilot Vorname"               , field_flight_pilot_first_name      ));	// Pilot Vorname
	fields_flight_db_entry.push_back (OOF ("Pilot Verein"                , field_flight_pilot_club            ));	// Pilot Verein
	fields_flight_db_entry.push_back (OOF ("Pilot VID"                   , field_flight_pilot_club_id         ));	// Pilot VID
	fields_flight_db_entry.push_back (OOF ("Begleiter Nachname"          , field_flight_copilot_last_name     ));	// Begleiter Nachname
	fields_flight_db_entry.push_back (OOF ("Begleiter Vorname"           , field_flight_copilot_first_name    ));	// Begleiter Vorname
	fields_flight_db_entry.push_back (OOF ("Begleiter Verein"            , field_flight_copilot_club          ));	// Begleiter Verein
	fields_flight_db_entry.push_back (OOF ("Begleiter VID"               , field_flight_copilot_club_id       ));	// Begleiter VID
	fields_flight_db_entry.push_back (OOF ("Flugtyp"                     , field_flight_flight_type           ));	// Flugtyp
	fields_flight_db_entry.push_back (OOF ("Anzahl Landungen"            , field_flight_num_landings          ));	// Anzahl Landungen
	fields_flight_db_entry.push_back (OOF ("Modus"                       , field_flight_mode                  ));	// Modus
	fields_flight_db_entry.push_back (OOF ("Startzeit"                   , field_flight_starttime             ));	// Startzeit
	fields_flight_db_entry.push_back (OOF ("Landezeit"                   , field_flight_landtime              ));	// Landezeit
	fields_flight_db_entry.push_back (OOF ("Flugdauer"                   , field_flight_duration              ));	// Flugdauer
	fields_flight_db_entry.push_back (OOF ("Startart"                    , field_flight_startart              ));	// Startart
	fields_flight_db_entry.push_back (OOF ("Kennzeichen Schleppflugzeug" , field_flight_registration_towplane ));	// Kennzeichen Schleppflugzeug
	fields_flight_db_entry.push_back (OOF ("Modus Schleppflugzeug"       , field_flight_mode_towplane         ));	// Modus Schleppflugzeug
	fields_flight_db_entry.push_back (OOF ("Landung Schleppflugzeug"     , field_flight_landtime_towplane     ));	// Landung Schleppflugzeug
	fields_flight_db_entry.push_back (OOF ("Startort"                    , field_flight_startort              ));	// Startort
	fields_flight_db_entry.push_back (OOF ("Zielort"                     , field_flight_zielort               ));	// Zielort
	fields_flight_db_entry.push_back (OOF ("Zielort Schleppflugzeug"     , field_flight_zielort_towplane      ));	// Zielort Schleppflugzeug
	fields_flight_db_entry.push_back (OOF ("Bemerkungen"                 , field_flight_comments              ));	// Bemerkungen
	fields_flight_db_entry.push_back (OOF ("Abrechnungshinweis"          , field_flight_abrechnungshinweis    ));	// Abrechnungshinweis
	fields_flight_db_entry.push_back (OOF ("DBID"                        , field_flight_id                    ));	// DBID

#undef CGI_SEL
#undef WEB_CHPW
#undef OF
}
/*}}}*/

what_next read_options ()/*{{{*/
{
	// Read the options
	// Read the options from the file. This is needed for various
	// things, like accessing the database.
	if (opts.read_config_files (&db, NULL, 0, NULL))
	{
		// Reading the options succeeded.
		// Now, we need to enter the data from the options read to
		// the db class.
		// For the user name and password, we use empty values for now because
		// we don't know yet which kind of login we're going to need.
		db.set_connection_data (opts.server, opts.port, "", "");
		db.set_database (opts.database);

		// Now we have dba_access
		session_access=dba_access;
	}
	else
	{
		// Reading the options failed.
		// We think that this is so bad that we fail right away.
		return what_next::output_error ("Konfigurationsdatei konnte nicht gelesen werden.");
	}

	return what_next::go_on ();
}
/*}}}*/

what_next setup_variables ()/*{{{*/
	/*
	 * Set up global variables needed for the web interface state handling system.
	 * These include:
	 *   - environment variables of the CGI interface and variables derived
	 *     from these, like the link to this script (variables).
	 *   - parameters passed by the user via the CGI interface (in cgi_args)
	 *     and variables derived from these, like the state we're going to.
	 *   - the session, if a session parameter is set.
	 *   - the current program state, in terms of a what_next object.
	 *   - the request method. The request method is set to rm_commandline if
	 *     no CGI environment is detected.
	 * Return value:
	 *   - a what_next object indicating what to do next.
	 */
{
	// First of all: silence on stdout./*{{{*/
	options::silent=true;
	db.silent=true;
/*}}}*/

	// Initialize variables/*{{{*/
	session_access=dba_none;
	session_user=NULL;
/*}}}*/

	// Read the options/*{{{*/
	DO_SUB_ACTION (read_options ());
/*}}}*/

	// Set up miscellaneous variables from the environment/*{{{*/
	remote_address=get_environment ("REMOTE_ADDR");
	client_is_local=opts.address_is_local (remote_address);
/*}}}*/

	// Set up URLs/*{{{*/
	// This is the URL we have to use in outputting links so the user can reach
	// this program. It is constructed from information found in the
	// environment.
	string script_name=get_environment ("SCRIPT_NAME");
	string http_host=get_environment ("HTTP_HOST");	// Already contains the port, if not default!
	string server_port=get_environment ("SERVER_PORT");

	string https_string=get_environment ("HTTPS");
	bool https_on=(https_string=="on" || https_string=="ON");
	string protocol_prefix=https_on?"https://":"http://";

//	int port_num=atoi (server_port.c_str ());
//	int default_port_num=https_on?443:80;
//	string url_server_port;
//
//	if (port_num==default_port_num)
//		url_server_port="";
//	else
//		url_server_port=":"+server_port;

	relative_url=script_name;
	absolute_url=protocol_prefix+http_host+script_name;
/*}}}*/

	// Build the cgi argument list./*{{{*/
	// These are arguments passed by the user (i. e., from the user-filled web
	// form). They are taken from the environment or from stdin, depending on
	// the request method found in the environment. The format is determined
	// from environment variables.
	// TODO:
	//   - move to the argument_list class
	//   - create cgi_argument_list class derived from argument_list
	//   - handle that funny multipart document type
	// GATEWAY_INTERFACE="CGI/1.1"
	// CONTENT_LENGTH
	// TODO auslagern!
	string method=get_environment ("REQUEST_METHOD");
	if (method=="GET" || method=="get" || method.empty ())
	{
		request_method=rm_get;
		if (method.empty ())
		{
			// No request method specified. This is probably the program bein run
			// outside of a CGI environment for debugging purposes. Assume GET.
			cerr << "Error: unknown request method \"" << method << "\", assuming GET." << endl;
			cerr << "For GET queries:" << endl;
			cerr << "export REQUEST_METHOD=\"GET\"" << endl << "export QUERY_STRING=\"action=login\"" << endl;
			cerr << "For POST queries:" << endl;
			cerr << "export REQUEST_METHOD=\"POST\"" << endl << "export CONTENT_TYPE=" << http_document::mime_type_form_urlencoded << endl << "export QUERY_STRING=\"action=login\"" << endl;
			cerr << "For CMDLINE queries:" << endl;
			cerr << "export REQUEST_METHOD=\"CMDLINE\"" << endl << "export QUERY_STRING=\"date_spec=today&action=do_flight_db\"" << endl << "export REMOTE_ADDR=127.0.0.1" << endl;
		}

		// Request method GET, data is passed via QUERY_STRING
		query_string=get_environment ("QUERY_STRING");
		cgi_args=argument_list::from_cgi_query (query_string);
	}
	else if (method=="POST" || method=="post")
	{
		request_method=rm_post;
		// Request method POST, data is passed on stdin
		// Format as determined by CONTENT_TYPE
		string content_type_string=get_environment ("CONTENT_TYPE");
		if (content_type_string.empty ()) return what_next::output_error ("CONTENT_TYPE f�r POST nicht angegeben");
		// Make a mime header structure
		mime_header content_type_header (content_type_string, mime_header::text_name_content_type);

		if (content_type_header.value==http_document::mime_type_form_urlencoded)
		{
			// Data is a query string
			getline (cin, query_string);
			cgi_args=argument_list::from_cgi_query (query_string);
		}
		else if (content_type_header.value==http_document::mime_type_multipart_form_data)
		{
			// Data is multipart. Now comes the fun part.
			if (content_type_header.args.has_argument ("boundary"))
			{
				// Boundary found
				string boundary=content_type_header.args.get_value ("boundary");

				string line;
				bool reading_header=false;
				bool reading_data=false;
				string current_key;
				string current_val;
				string current_filename;

				while (cin.good ())
				{
					// Read a line from stdin
					getline (cin, line);
					if (!line.empty () && line.at (line.length ()-1)=='\r')
						line=line.substr (0, line.length ()-1);

					debug_stream << "[" << line << "]" << endl;

					if (starts_with (line, "--"+boundary))
					{
						// We found a boundary. This means that the current part is over.

						// If we have read data, add it to the list.
						if (reading_data && !current_key.empty ())
							cgi_args.set_value (current_key, current_val);

						if (!current_filename.empty ())
							filenames.set_value (current_key, current_filename);
						current_filename.clear ();

						if (line=="--"+boundary+"--")
						{
							// End delimiter. Stop reading.
							break;
						}
						else
						{
							// New header
							reading_header=true;
							reading_data=false;
							current_key="";
							current_val="";
						}
					}
					else if (reading_header)
					{
						// We are currently reading a part header.
						if (trim (line).empty ())
						{
							// Empty line. This means that this is the end of the header.

							// If no name was set, this is an error.
							if (current_key.empty ()) return what_next::output_error ("Kein Elementname im Header angegeben");

							reading_header=false;
							reading_data=true;
						}
						else
						{
							// Header line, parse it.
							mime_header part_header (line);
							if (part_header.name==mime_header::text_name_content_disposition)
							{
								// Content-disposition header. This carries the name of the element.

								// Check that the content-disposition is form-data
								if (part_header.value!="form-data") return what_next::output_error ("Unbekannte Content-Disposition: "+part_header.value);

								// Check and save the element name
								if (!part_header.args.has_argument ("name")) return what_next::output_error ("Content-Disposition enth�lt keinen Namen");
								current_key=part_header.args.get_value ("name");

								current_filename=part_header.args.get_value ("filename");
							}
						}
					}
					else if (reading_data)
					{
						// Data line, add it.
						if (!current_val.empty ()) current_val+="\n";
						current_val+=line;
					}
				}
			}
			else
			{
				// No boundary found
				return what_next::output_error ("Kein \"boundary\"-Eintrag gefunden");
			}
		}
		else
		{
			return what_next::output_error ("Unbekannter content_type \""+content_type_header.value+"\"");
		}
	}
	else if (method=="CMDLINE" || method=="cmdline")
	{
		request_method=rm_commandline;

		// Data is read from QUERY_STRING, as with request method GET.
		query_string=get_environment ("QUERY_STRING");
		cgi_args=argument_list::from_cgi_query (query_string);
	}
	else
	{
		request_method=rm_other;
		return what_next::output_error ("Unsupported request method \""+method+"\"");
	}
/*}}}*/

	// Set up some global variables from CGI arguments/*{{{*/
	debug_enabled=CGI_HAS (debug);
/*}}}*/

	// Set up the session/*{{{*/
	// If a session_id CGI parameter was given, we try to open the session,
	// regardless of whether the state we're going to needs a session. If
	// opening the session failes, an error is output (returned). After opening
	// the session, we check it for validity (e. g., the remote address
	// matches). If it is not valid, an error is returned.
	// If not session_id is given, we continue without a session.
	// The session validity can be determined by using the is_ok method of the
	// session.
	if (CGI_HAS (session_id))
	{
		session=web_session::open (CGI_READ (session_id));
		if (session.is_ok ())
		{
			// Check the session for validity.
			if (!SESSION_HAS (remote_address))
				return what_next::output_error ("client-Adresse nicht bekannt (Programmfehler)");

			string session_remote_address=SESSION_READ (remote_address);
			if (remote_address!=session_remote_address)
				return what_next::output_error ("Falsche client-Adresse (ist: \""+remote_address+"\"; soll: \""+session_remote_address+"\")");
		}
		else
		{
			// Session ain't no good.
			// Cannot use error_description of the session because that only
			// works for creation.
			return what_next::output_error ("Ung�ltige Sitzung \""+CGI_READ (session_id)+"\"");
		}
	}
/*}}}*/

	// Set the database connection data if there was a connection./*{{{*/
	if (session.is_ok ())
	{
		if (SESSION_HAS (login_name))
		{
			session_username=SESSION_READ (login_name);
			if (session_username.empty ()) return what_next::output_error ("Leerer Benutzername");

			user_class_t user_class=determine_user_class (session_username);
			switch (user_class)
			{
				case uc_none:
				{
					// User which cannot log in.
					// How did he make it into the session?
					// No change in session_access
					return what_next::output_error ("Benutzer mit Benutzerklasse uc_none in session");
				} break;
				case uc_mysql_user:
				{
					// MySQL user. Password must also have been saved.
					string password=SESSION_READ (password);
					db.set_user_data (session_username, password);

					if (session_username==opts.root_name)
						session_access=dba_root;
					else if (session_username==opts.sk_admin_name)
						session_access=dba_sk_admin;
					else
						return what_next::output_error ("Unbehandelter MySQL-Benutzer (Programmfehler)");
				} break;
				case uc_sk_user:
				{
					// sk user. Password is not saved because sk_admin is used for
					// MySQL login. There is a user ==> login was successful ==>
					// the sk_admin data in the configuration were correct.

					if (opts.sk_admin_name.empty ()) return what_next::output_error ("sk_admin nicht mehr definiert");
					if (opts.sk_admin_password.empty ()) return what_next::output_error ("sk_admin_password nicht mehr definiert");

					db.set_is_admin_db (true);
					db.set_user_data (opts.sk_admin_name, opts.sk_admin_password);

					// ...==> the user is authenticated.
					session_access=dba_sk_user;

					// Cannot set session_user right now because the
					// database connection is not established yet.
				} break;
			}
		}
	}
	else
	{
		// There is no session. In case we need database access, use the
		// sk_admin user.

		if (opts.sk_admin_name.empty ()) return what_next::output_error ("sk_admin nicht mehr definiert");
		if (opts.sk_admin_password.empty ()) return what_next::output_error ("sk_admin_password nicht mehr definiert");

		db.set_is_admin_db (true);
		db.set_user_data (opts.sk_admin_name, opts.sk_admin_password);


		if (request_method==rm_commandline)
		{
			// There is unlimited access to the database.
			session_access=dba_sk_admin;
		}
		else
		{
			// DB access is present.
			// Don't set dba_sk_admin here because this variable is used for
			// authorization!
			session_access=dba_access;
		}
	}

/*}}}*/

	// Determine and return the state./*{{{*/
	// If a new_state cgi argument is given, this state is to be used. If not,
	// we use the default state, which is probably either login or some kind of
	// public main menu.
	if (CGI_HAS (new_state))
	{
		// A state was given, use it.
		string state_string=CGI_READ (new_state);
		try
		{
			web_interface_state::from_list (state_string);
			return what_next::go_to_state (state_string);
		}
		catch (web_interface_state::ex_not_found)
		{
			return what_next::output_error ("Ung�ltiger Zustand \""+state_string+"\"");
		}
	}
	else
	{
		// No state was given, use the default
		return what_next::go_to_state (default_state);
	}
/*}}}*/
}
/*}}}*/

int main (int argc, char *argv[])/*{{{*/
{
//	table_row::test (); return 0;
//	table::test (); return 0;
//	mime_header::test (); return 0;
//	sk_db::test (); return 0;

	// Initialize static and dynamic data
	setup_static_data ();
	what_next next=setup_variables ();

//	if (request_method==rm_commandline)
//	{
//	}
//	else
//	{
		// Execute the states
		http_document http;
		if (request_method==rm_commandline) http.no_header=true;

		while (next.get_next ()!=wn_end_program) next=do_next (next, http);

		// Save the session
		if (session.is_ok ()) session.save ();

		// Output the document, *after* the session has been saved. This is
		// required to avoid a race condition where the next page is loaded before
		// the session has been saved completely.

		http.do_output (cout);
//	}

	return 0;
}
/*}}}*/


