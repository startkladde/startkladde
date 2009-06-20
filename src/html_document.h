#ifndef html_document_h
#define html_document_h

/*
 * html_document
 * martin
 * 2005-01-01
 */

#include <string>
#include "accessor.h"
#include <sstream>
#include "text.h"	// TODO move html_escape here?
#include "argument.h"

using namespace std;

/*
 * Thoughts about a successor:
 *   - Find a clear and consistent way to mark text that still has to be
 *     escaped.
 */

class html_table_cell 
{
	public:
		html_table_cell (const string &_contents="", bool _header=false, int _colspan=1):
			contents (_contents),
			header (_header),
			colspan (_colspan)
			{}
		// A colspan of 0 means "rest of the table". A colspan of 1 is not
		// explicitly output.
		static html_table_cell text (const string &_text, bool _header=false, int _colspan=1) { return html_table_cell (html_escape (_text), _header, _colspan); }
		static html_table_cell empty (int _colspan=1) { return html_table_cell ("", false, _colspan); }

		string contents;
		bool header;
		unsigned int colspan;
};

class html_table_row:public list<html_table_cell>
{
	public:
		unsigned int num_cells () const;
};

typedef list<html_table_row> html_table;


class html_document
{
	public:
		// TODO separate
		//   - writing html head
		//   - writing the initial caption
		html_document (bool _auto_title=true);
		html_document &start_section (const string &caption);
		html_document &end_section ();
		html_document &write (const string &s, bool do_indent=true);
		html_document &write_text (const string &s, bool do_indent=true);
		html_document &write_text (const char *s, bool do_indent=true) { return write_text (string (s), do_indent); }
		html_document &write_text (bool b, bool do_indent=true);
		html_document &write_paragraph (const string &s, const string &style_class="");
		html_document &write_error_paragraph (const string &s);
		html_document &write_hr () { write ("<hr>\n"); return *this; }
		html_document &write_br () { write ("<br>\n"); return *this; }
		html_document &write_anchor (const string &name, const string &text="") { return write (anchor (name, text)); }
		html_document &write_text_link (const string &target, const string &text) { return write (text_link (target, text)); }
		html_document &write_list (const list<string> &entries, bool numbered=false);
		html_document &write_text_list (const list<string> &entries, bool numbered=false);
		html_document &start_tag (const string &tag, const string &params="");
		html_document &end_tag (const string &tag);
		html_document &start_paragraph (const string &style_class="");
		html_document &end_paragraph ();
		html_document &write_hidden_field (const string &name, const string &value="");
		html_document &write_hidden_fields (const argument_list &args);
		html_document &write_hidden_fields (const list<argument> &args);
		html_document &endl () { write ("\n"); return *this; }
		html_document &write_input_text (const string &name, const string &value="", unsigned int size=0);
		html_document &write_input_file (const string &name);
		html_document &write_input_password (const string &name, const string &value="");
		html_document &write_submit (const string &caption);
		html_document &write_input_checkbox (const string &name, const string &text, bool checked=false);
		html_document &write_input_radio (const string &name, const string &value, const string &text, bool checked=false);
		html_document &write_input_select (const string &name, const argument_list &options, const string &selected="");
		html_document &write_input_select_bool (const string &name, bool is_selected);
		html_document &write_table_data (const string &text, int colspan=1);
		html_document &write (const html_table_cell &cell, unsigned int num, unsigned int num_columns);
		html_document &write (const html_table_row &row, unsigned int num_columns);
		html_document &write (const html_table &table, bool noborder=false);
		html_document &write_text_button (const string &name, const string &value, const string &text);
		html_document &write_preformatted (const string &text);

		string make_input_text (const string &name, const string &value="", unsigned int size=0);
		string make_input_file (const string &name);
		string make_input_password (const string &name, const string &value="");
		string make_submit (const string &caption);
		string make_input_checkbox (const string &name, const string &text, bool checked=false);
		string make_input_radio (const string &name, const string &value, const string &text, bool checked=false);
		string make_input_select (const string &name, const argument_list &options, const string &selected="");
		string make_input_select_bool (const string &name, bool is_selected);
		string make_text_button (const string &name, const string &value, const string &text);
		string make_list (const list<string> &entries, bool numbered=false);
		string make_text_list (const list<string> &entries, bool numbered=false);

		void clear ();
		string text () const;
		RW_ACCESSOR (string, title)
		static string param (const string &name, const string &value);
		static string text_link (const string &target, const string &text);
		static string text_link (const string &target, const string &text, const argument_list &args);
		static string anchor (const string &name, const string &text="");

	private:
		string charset;
		ostringstream out;
		bool newline;	// A new line was started
		bool auto_title;
		int indent;
		int heading;
		string title;
};

#endif

