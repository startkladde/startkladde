#ifndef object_field_h
#define object_field_h

/*
 * object_field
 * martin
 * 2005-01-04
 */

#include <string>

#include "src/accessor.h"
#include "src/db/db_types.h"

using namespace std;

/*
 * This is still not satisfying.
 * Situations where it is insufficient:
 *   - in the users table, in the person column, the name of the user should be
 *     displayed.
 *   - in the users table, in the person column, there should be a link to the
 *     state displaying details about that person.
 * Of course, it is not a good solution that this class carries both data and
 * metadata.
 * Another thing to consider when rewriting this is that there are many
 * possible ways to display/hide data: not all columns may be shown, some
 * columns may or may not have a link, some might not be editable (although
 * generally, they are) etc.
 * It may also be required that any arbitrary field (for example "error" for
 * master_data_check) be added to the list.
 * There should be a way to specify no-break for columns.
 * Another thing: for LaTeX tables, a column width might be needed. For other
 * table types, other things might be needed (no-break coule be unified into
 * this).  Using inheritance, you should be able to specify such things. Might
 * this replace the table class (at all)?
 */

/*
 * More notes:
 *   - Must have a function for finding from list.
 *   - Need value for date (handled differently in LaTeX/CSV)
 *   - Need value for "none", gets "" in CSV, "---" in LaTeX
 *   - Maybe this should also be used for passing parameters, like for getting
 *     logbooks, where there is a date mode, a date, a flight instructor mode
 *     etc.
 *   - Note that in sk_web, we define a list of object_fields for output to CSV
 *     tables and we have to define all of the editing and creating properties
 *     although they are never used. This is not very good.
 *     Now, with output_field, they do not need to be specified, but they are
 *     still carried around.
 *   - The information about field widths should probably be part of this
 *     structure.
 */

class object_field
{
	public:
		enum data_type_t { dt_string, dt_db_id, dt_bool, dt_password, dt_special };

		object_field (
			const string &_caption,
			data_type_t _data_type,
			bool _list_display,
			bool _edit_display,
			bool _edit_edit,
			const string &_edit_state,
			const string &_edit_state_caption,
			bool _create_display,
			bool _create_edit,
			const string &_create_state,
			const string &_create_state_caption,
			const string &_label
			):
			caption (_caption),
			data_type (_data_type),
			list_display (_list_display),
			edit_display (_edit_display),
			edit_edit (_edit_edit),
			edit_state (_edit_state),
			edit_state_caption (_edit_state_caption),
			create_display (_create_display),
			create_edit (_create_edit),
			create_state (_create_state),
			create_state_caption (_create_state_caption),
			label (_label),
			given (false),
			locked (false),
			no_break (false)
			{};

		static object_field output_field (const string &_caption, data_type_t _data_type, const string &_label)
		{
			return object_field (_caption, _data_type, true, true, false, "", "", true, false, "", "", _label);
		}
		static object_field output_field (const string &_caption, const string &_label)
		{
			return object_field (_caption, dt_string, true, true, false, "", "", true, false, "", "", _label);
		}

		// MURX: field description (metadata) and data in same class

		// These functions set the value saved to a given value.
		void clear ();
		void set_to (const string &string_value) { current_string=string_value; }
		void set_to (const char *string_value) { set_to (string (string_value)); }
		void set_to (db_id db_id_value) { current_db_id=db_id_value; }
		void set_to (bool bool_value) { current_bool=bool_value; }

		// These functions get the value saved.
		string get_string () const { return current_string; }
		db_id get_db_id () const { return current_db_id; }
		bool get_bool () const { return current_bool; }

		// This function parses a text representation (for example, from a CGI
		// form)
		void parse_text (const string &text);

		// This function makes a string that can be parsed (for example, for
		// presetting a CGI form)
		string make_text () const;
		string make_text_safe () const;

		// This function makes a string suitable for display in a table
		string make_display_text () const;

		bool list_present () const { return list_display; }
		bool edit_present () const { return edit_display || edit_edit || !edit_state.empty (); }
		bool create_present () const { return create_display || create_edit || !create_state.empty (); }

		RO_ACCESSOR (string, caption)
		RO_ACCESSOR (data_type_t, data_type)
		RO_ACCESSOR (bool, list_display)
		RO_ACCESSOR (bool, edit_display)
		RO_ACCESSOR (bool, edit_edit)
		RO_ACCESSOR (string, edit_state)
		RO_ACCESSOR (string, edit_state_caption)
		RO_ACCESSOR (bool, create_display)
		RO_ACCESSOR (bool, create_edit)
		RO_ACCESSOR (string, create_state)
		RO_ACCESSOR (string, create_state_caption)
		RO_ACCESSOR (string, label)

		RW_ACCESSOR (bool, given)
		RW_ACCESSOR (bool, locked)

		RW_ACCESSOR_REF_DEF (object_field, bool, no_break, true);

	private:
		// Field description in general
		string caption;
		data_type_t data_type;

		bool list_display;

		bool edit_display;
		bool edit_edit;
		string edit_state;
		string edit_state_caption;

		bool create_display;
		bool create_edit;
		string create_state;
		string create_state_caption;

		string label;

		// Current value
		string current_string;
		db_id current_db_id;
		bool current_bool;

		bool given;

		bool locked;

		bool no_break;
};

#endif

