#ifndef _HtmlDocument_h
#define _HtmlDocument_h

#include <QString>
#include <sstream>

#include "src/accessor.h"
#include "src/text.h"	// TODO move html_escape here?
#include "src/web/Argument.h"

/*
 * Thoughts about a successor:
 *   - Find a clear and consistent way to mark text that still has to be
 *     escaped.
 */

class HtmlTableCell
{
	public:
		HtmlTableCell (const QString &_contents="", bool _header=false, int _colspan=1):
			contents (_contents),
			header (_header),
			colspan (_colspan)
			{}
		// A colspan of 0 means "rest of the Table". A colspan of 1 is not
		// explicitly output.
		static HtmlTableCell text (const QString &_text, bool _header=false, int _colspan=1) { return HtmlTableCell (html_escape (_text), _header, _colspan); }
		static HtmlTableCell empty (int _colspan=1) { return HtmlTableCell ("", false, _colspan); }

		QString contents;
		bool header;
		unsigned int colspan;
};

class HtmlTableRow:public QList<HtmlTableCell>
{
	public:
		unsigned int num_cells () const;
};

typedef QList<HtmlTableRow> html_table;


class HtmlDocument
{
	public:
		// TODO separate
		//   - writing html head
		//   - writing the initial caption
		HtmlDocument (bool _auto_title=true);
		HtmlDocument &start_section (const QString &caption);
		HtmlDocument &end_section ();
		HtmlDocument &write (const QString &s, bool do_indent=true);
		HtmlDocument &write_text (const QString &s, bool do_indent=true);
		HtmlDocument &write_text (const char *s, bool do_indent=true) { return write_text (QString (s), do_indent); }
		HtmlDocument &write_text (bool b, bool do_indent=true);
		HtmlDocument &write_paragraph (const QString &s, const QString &style_class="");
		HtmlDocument &write_error_paragraph (const QString &s);
		HtmlDocument &write_hr () { write ("<hr>\n"); return *this; }
		HtmlDocument &write_br () { write ("<br>\n"); return *this; }
		HtmlDocument &write_anchor (const QString &name, const QString &text="") { return write (anchor (name, text)); }
		HtmlDocument &write_text_link (const QString &target, const QString &text) { return write (text_link (target, text)); }
		HtmlDocument &write_list (QStringList entries, bool numbered=false);
		HtmlDocument &write_text_list (QStringList entries, bool numbered=false);
		HtmlDocument &start_tag (const QString &tag, const QString &params="");
		HtmlDocument &end_tag (const QString &tag);
		HtmlDocument &start_paragraph (const QString &style_class="");
		HtmlDocument &end_paragraph ();
		HtmlDocument &write_hidden_field (const QString &name, const QString &value="");
		HtmlDocument &write_hidden_fields (const ArgumentList &args);
		HtmlDocument &write_hidden_fields (const QList<Argument> &args);
		HtmlDocument &endl () { write ("\n"); return *this; }
		HtmlDocument &write_input_text (const QString &name, const QString &value="", unsigned int size=0);
		HtmlDocument &write_input_file (const QString &name);
		HtmlDocument &write_input_password (const QString &name, const QString &value="");
		HtmlDocument &write_submit (const QString &caption);
		HtmlDocument &write_input_checkbox (const QString &name, const QString &text, bool checked=false);
		HtmlDocument &write_input_radio (const QString &name, const QString &value, const QString &text, bool checked=false);
		HtmlDocument &write_input_select (const QString &name, const ArgumentList &options, const QString &selected="");
		HtmlDocument &write_input_select_bool (const QString &name, bool is_selected);
		HtmlDocument &write_table_data (const QString &text, int colspan=1);
		HtmlDocument &write (const HtmlTableCell &cell, unsigned int num, unsigned int num_columns);
		HtmlDocument &write (const HtmlTableRow &row, unsigned int num_columns);
		HtmlDocument &write (const html_table &table, bool noborder=false);
		HtmlDocument &write_text_button (const QString &name, const QString &value, const QString &text);
		HtmlDocument &write_preformatted (const QString &text);

		QString make_input_text (const QString &name, const QString &value="", unsigned int size=0);
		QString make_input_file (const QString &name);
		QString make_input_password (const QString &name, const QString &value="");
		QString make_submit (const QString &caption);
		QString make_input_checkbox (const QString &name, const QString &text, bool checked=false);
		QString make_input_radio (const QString &name, const QString &value, const QString &text, bool checked=false);
		QString make_input_select (const QString &name, const ArgumentList &options, const QString &selected="");
		QString make_input_select_bool (const QString &name, bool is_selected);
		QString make_text_button (const QString &name, const QString &value, const QString &text);
		QString make_list (QStringList entries, bool numbered=false);
		QString make_text_list (QStringList entries, bool numbered=false);

		void clear ();
		QString text () const;
		RW_ACCESSOR (QString, title)
		static QString param (const QString &name, const QString &value);
		static QString text_link (const QString &target, const QString &text);
		static QString text_link (const QString &target, const QString &text, const ArgumentList &args);
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

