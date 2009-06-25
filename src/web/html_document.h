#ifndef html_document_h
#define html_document_h

/*
 * html_document
 * martin
 * 2005-01-01
 */

#include <QString>
#include <sstream>

#include "src/accessor.h"
#include "src/text.h"	// TODO move html_escape here?
#include "src/web/argument.h"

/*
 * Thoughts about a successor:
 *   - Find a clear and consistent way to mark text that still has to be
 *     escaped.
 */

class html_table_cell
{
	public:
		html_table_cell (const QString &_contents="", bool _header=false, int _colspan=1):
			contents (_contents),
			header (_header),
			colspan (_colspan)
			{}
		// A colspan of 0 means "rest of the table". A colspan of 1 is not
		// explicitly output.
		static html_table_cell text (const QString &_text, bool _header=false, int _colspan=1) { return html_table_cell (html_escape (_text), _header, _colspan); }
		static html_table_cell empty (int _colspan=1) { return html_table_cell ("", false, _colspan); }

		QString contents;
		bool header;
		unsigned int colspan;
};

class html_table_row:public std::list<html_table_cell>
{
	public:
		unsigned int num_cells () const;
};

typedef std::list<html_table_row> html_table;


class html_document
{
	public:
		// TODO separate
		//   - writing html head
		//   - writing the initial caption
		html_document (bool _auto_title=true);
		html_document &start_section (const QString &caption);
		html_document &end_section ();
		html_document &write (const QString &s, bool do_indent=true);
		html_document &write_text (const QString &s, bool do_indent=true);
		html_document &write_text (const char *s, bool do_indent=true) { return write_text (QString (s), do_indent); }
		html_document &write_text (bool b, bool do_indent=true);
		html_document &write_paragraph (const QString &s, const QString &style_class="");
		html_document &write_error_paragraph (const QString &s);
		html_document &write_hr () { write ("<hr>\n"); return *this; }
		html_document &write_br () { write ("<br>\n"); return *this; }
		html_document &write_anchor (const QString &name, const QString &text="") { return write (anchor (name, text)); }
		html_document &write_text_link (const QString &target, const QString &text) { return write (text_link (target, text)); }
		html_document &write_list (QStringList entries, bool numbered=false);
		html_document &write_text_list (QStringList entries, bool numbered=false);
		html_document &start_tag (const QString &tag, const QString &params="");
		html_document &end_tag (const QString &tag);
		html_document &start_paragraph (const QString &style_class="");
		html_document &end_paragraph ();
		html_document &write_hidden_field (const QString &name, const QString &value="");
		html_document &write_hidden_fields (const argument_list &args);
		html_document &write_hidden_fields (const std::list<argument> &args);
		html_document &endl () { write ("\n"); return *this; }
		html_document &write_input_text (const QString &name, const QString &value="", unsigned int size=0);
		html_document &write_input_file (const QString &name);
		html_document &write_input_password (const QString &name, const QString &value="");
		html_document &write_submit (const QString &caption);
		html_document &write_input_checkbox (const QString &name, const QString &text, bool checked=false);
		html_document &write_input_radio (const QString &name, const QString &value, const QString &text, bool checked=false);
		html_document &write_input_select (const QString &name, const argument_list &options, const QString &selected="");
		html_document &write_input_select_bool (const QString &name, bool is_selected);
		html_document &write_table_data (const QString &text, int colspan=1);
		html_document &write (const html_table_cell &cell, unsigned int num, unsigned int num_columns);
		html_document &write (const html_table_row &row, unsigned int num_columns);
		html_document &write (const html_table &table, bool noborder=false);
		html_document &write_text_button (const QString &name, const QString &value, const QString &text);
		html_document &write_preformatted (const QString &text);

		QString make_input_text (const QString &name, const QString &value="", unsigned int size=0);
		QString make_input_file (const QString &name);
		QString make_input_password (const QString &name, const QString &value="");
		QString make_submit (const QString &caption);
		QString make_input_checkbox (const QString &name, const QString &text, bool checked=false);
		QString make_input_radio (const QString &name, const QString &value, const QString &text, bool checked=false);
		QString make_input_select (const QString &name, const argument_list &options, const QString &selected="");
		QString make_input_select_bool (const QString &name, bool is_selected);
		QString make_text_button (const QString &name, const QString &value, const QString &text);
		QString make_list (QStringList entries, bool numbered=false);
		QString make_text_list (QStringList entries, bool numbered=false);

		void clear ();
		QString text () const;
		RW_ACCESSOR (QString, title)
		static QString param (const QString &name, const QString &value);
		static QString text_link (const QString &target, const QString &text);
		static QString text_link (const QString &target, const QString &text, const argument_list &args);
		static QString anchor (const QString &name, const QString &text="");

	private:
		QString charset;
		// TODO remove global output buffer
		std::ostringstream out;
		bool newline;	// A new line was started
		bool auto_title;
		int indent;
		int heading;
		QString title;
};

#endif

