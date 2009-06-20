#include "html_document.h"

unsigned int html_table_row::num_cells () const/*{{{*/
	// Returns the number of cells in the row, according to their colspan
{
	int r=0;

	const_iterator e=end ();
	for (const_iterator it=begin (); it!=e; ++it)
	{
		if ((*it).colspan==0)
			r+=1;
		else
			r+=(*it).colspan;
	}

	return r;
}
/*}}}*/




html_document::html_document (bool _auto_title)/*{{{*/
	:newline (true),
	auto_title (_auto_title),
	indent (auto_title?2:0),
	heading (auto_title?2:1)
{

}
/*}}}*/

html_document &html_document::write (const string &s, bool do_indent)/*{{{*/
{
	string indent_string;
	if (do_indent)
		indent_string=string (2*indent, ' ');
	else
		indent_string="";

	string::const_iterator end=s.end ();
	for (string::const_iterator ch=s.begin (); ch!=end; ++ch)
	{
		// Indentation after a new line started
		if (newline) out << indent_string;
		newline=false;

		if (*ch=='\n') newline=true;

		out << *ch;
	}

	return *this;
}
/*}}}*/

html_document &html_document::write_text (const string &s, bool do_indent)/*{{{*/
{
	return write (html_escape (s), do_indent);
}
/*}}}*/

html_document &html_document::write_text (bool b, bool do_indent)/*{{{*/
{
	return write (html_escape (bool_to_string (b)), do_indent);
}
/*}}}*/

html_document &html_document::write_paragraph (const string &s, const string &style_class)/*{{{*/
{
	string style_string;
	if (!style_class.empty ()) style_string=" class=\""+style_class+"\"";

	if (s.find ('\n')!=string::npos)
	{
		// There is a newline, write a multi line paragraph
		start_tag ("p", style_string);
		write (s);
		end_tag ("p");
	}
	else
	{
		// There is no newline, write a single line paragraph
		write ("<p"+style_string+">"+s+"</p>\n");
	}

	return *this;
}
/*}}}*/

html_document &html_document::write_error_paragraph (const string &s)/*{{{*/
{
	return write_paragraph (s, "error");
}
/*}}}*/

void html_document::clear ()/*{{{*/
{
	out.str ("");
	title="";
}
/*}}}*/

string html_document::text () const/*{{{*/
{
	if (auto_title)
	{
		return
		"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"
		"<html>\n"
		"  <head>\n"
		"    <title>"+html_escape (title)+"</title>\n"
		// TODO
//		"    <link rev=\"made\" href=\"mailto:martin.herrmann@stud.uni-karlsruhe.de\">\n"
		"    <link rel=\"stylesheet\" type=\"text/css\" href=\"sk_web.css\">\n"
		// TODO synchronize this charset with the http header charset
		"    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>"+html_escape (title)+"</h1>\n"
		+out.str ()+
		"  </body>\n"
		"</html>\n"
		;
	}
	else
	{
		return out.str ();
	}
}
/*}}}*/

html_document &html_document::start_section (const string &caption)/*{{{*/
{
	write (
		"<h"+num_to_string (heading)+">"
		+caption+
		"</h"+num_to_string (heading)+">\n"
		);
	++heading;

	return *this;
}
/*}}}*/

html_document &html_document::end_section ()/*{{{*/
{
	--heading;

	return *this;
}
/*}}}*/

html_document &html_document::start_paragraph (const string &style_class)/*{{{*/
{
	string style_string;
	if (!style_class.empty ()) style_string=" class=\""+style_class+"\"";
	start_tag ("p", style_string);
	return *this;
}
/*}}}*/

html_document &html_document::end_paragraph ()/*{{{*/
{
	end_tag ("p");
	return *this;
}
/*}}}*/



string html_document::param (const string &name, const string &value)/*{{{*/
{
	return " "+name+"=\""+html_escape (value)+"\"";
}
/*}}}*/

string html_document::text_link (const string &target, const string &text)/*{{{*/
{
	return "<a href=\""+html_escape (target)+"\">"+html_escape (text)+"</a>";
}
/*}}}*/

string html_document::text_link (const string &target, const string &text, const argument_list &args)/*{{{*/
{
	string targ=target;
	if (!args.empty ()) targ+="?"+args.make_cgi_parameters ();

	return "<a href=\""+html_escape (targ)+"\">"+html_escape (text)+"</a>";
}
/*}}}*/

string html_document::anchor (const string &name, const string &text)/*{{{*/
{
	return "<a name=\""+html_escape (name)+"\">"+html_escape (text)+"</a>";
}
/*}}}*/

html_document &html_document::start_tag (const string &tag, const string &params)/*{{{*/
{
	string text=tag;
	if (!params.empty ()) text+=" "+params;
	write ("<"+text+">\n");
	++indent;
	return *this;
}
/*}}}*/

html_document &html_document::end_tag (const string &tag)/*{{{*/
{
	--indent;
	write ("</"+tag+">\n");
	return *this;
}
/*}}}*/

html_document &html_document::write_hidden_field (const string &name, const string &value)/*{{{*/
{
	return write ("<div><input type=\"hidden\" name=\""+html_escape (name)+"\" value=\""+html_escape (value)+"\"></div>\n");
}
/*}}}*/

html_document &html_document::write_hidden_fields (const list<argument> &args)/*{{{*/
{
	string r;
	list<argument>::const_iterator end=args.end ();
	for (list<argument>::const_iterator it=args.begin (); it!=end; ++it)
		write_hidden_field ((*it).get_name (), (*it).get_value ());

	return *this;
}
/*}}}*/

html_document &html_document::write_hidden_fields (const argument_list &args)/*{{{*/
{
	return write_hidden_fields (args.get_list ());
}
/*}}}*/

html_document &html_document::write_table_data (const string &text, int colspan)/*{{{*/
{
	write ("<td");
	if (colspan>1) write (" colspan="+num_to_string (colspan));
	write (">"+text+"</td>\n");
	return *this;
}
/*}}}*/



html_document &html_document::write (const html_table_cell &cell, unsigned int num, unsigned int num_columns)/*{{{*/
	// num: 0 based
{
	string tag=cell.header?"th":"td";
	string params;

	if (cell.colspan>1)
		params+=" colspan="+num_to_string (cell.colspan);
	else if (cell.colspan==0 && num_columns-num>1)
		params+=" colspan="+num_to_string (num_columns-num);

	params+=" valign=\"top\"";

	if (cell.contents.find ('\n')==string::npos)
	{
		// Single-line cell
		write ("<"+tag+params+">");
		write (cell.contents);
		write ("</"+tag+">\n");
	}
	else
	{
		// Multi-line cell
		write ("<"+tag+params+">\n");
		++indent;
		write (cell.contents);
		--indent;
		write ("</"+tag+">\n");
	}

	return *this;
}
/*}}}*/

html_document &html_document::write (const html_table_row &row, unsigned int num_columns)/*{{{*/
{
	start_tag ("tr");
	html_table_row::const_iterator end=row.end ();
	int num=0;	// The number of the cell we're writing

	for (html_table_row::const_iterator it=row.begin (); it!=end; ++it)
	{
		write (*it, num, num_columns);
		num++;
	}

	end_tag ("tr");
	return *this;
}
/*}}}*/

html_document &html_document::write (const html_table &table, bool noborder)/*{{{*/
{
	string attrib;
	if (noborder) attrib="class=\"noborder\"";
	start_tag ("table", attrib);
	html_table::const_iterator end=table.end ();
	// First, count the maximum number of cells, so that colspan==0 for "rest
	// of the table" works.
	unsigned int num_columns=0;
	for (html_table::const_iterator it=table.begin (); it!=end; ++it)
		if ((*it).size ()>num_columns)
			num_columns=(*it).num_cells ();

	// Now, write the table
	for (html_table::const_iterator it=table.begin (); it!=end; ++it)
		write (*it, num_columns);
	end_tag ("table");
	return *this;
}
/*}}}*/


string html_document::make_input_text (const string &name, const string &value, unsigned int size)/*{{{*/
{
	string params;
	params+=" name=\""+html_escape (name)+"\"";
	if (!value.empty ()) params+=" value=\""+html_escape (value)+"\"";
	if (size>0) params+=" size="+num_to_string (size);

	return "<input type=\"text\""+params+">\n";
}
/*}}}*/

string html_document::make_input_file (const string &name)/*{{{*/
{
	return "<input type=\"file\" name=\""+html_escape (name)+"\">\n";
}
/*}}}*/

string html_document::make_input_password (const string &name, const string &value)/*{{{*/
{
	string r;

	r+="<input type=\"password\" name=\""+html_escape (name)+"\"";
	if (!value.empty ()) r+=" value=\""+value+"\"";
	r+=">\n";

	return r;
}
/*}}}*/

string html_document::make_submit (const string &caption)/*{{{*/
{
	return "<input type=\"submit\" value=\""+html_escape (caption)+"\">\n";
}
/*}}}*/

string html_document::make_input_checkbox (const string &name, const string &text, bool checked)/*{{{*/
{
	string r;
	r="<input type=\"checkbox\" name=\""+html_escape (name)+"\"";
	if (checked) r+=" checked";
	r+=">"+html_escape (text)+"\n";
	return r;
}
/*}}}*/

string html_document::make_input_radio (const string &name, const string &value, const string &text, bool checked)/*{{{*/
{
	string r;
	r="<input type=\"radio\" name=\""+html_escape (name)+"\" value=\""+html_escape (value)+"\"";
	if (checked) r+=" checked";
	r+=">";
	if (!text.empty ()) r+=" "+html_escape (text);
	r+="\n";
	return r;
}
/*}}}*/

string html_document::make_input_select (const string &name, const argument_list &options, const string &selected)/*{{{*/
{
	string r;
	r+="<select name=\""+name+"\">\n";

	list<argument>::const_iterator end=options.get_list ().end ();
	for (list<argument>::const_iterator option=options.get_list ().begin (); option!=end; ++option)
	{
		string selected_string;
		if ((*option).get_name ()==selected) selected_string=" selected";
		r+="  <option"+selected_string+" value=\""+(*option).get_name ()+"\">"+(*option).get_value ()+"</option>\n";
	}
	r+="</select>\n";

	return r;
}
/*}}}*/

string html_document::make_input_select_bool (const string &name, bool selected_value)/*{{{*/
{
	argument_list options;
	options.set_value ("0", bool_to_string (false));
	options.set_value ("1", bool_to_string (true));

	string selected=selected_value?"1":"0";
	return make_input_select (name, options, selected);
}
/*}}}*/

string html_document::make_text_button (const string &name, const string &value, const string &text)/*{{{*/
{
	return "<button type=\"submit\" name=\""+html_escape (name)+"\" value=\""+html_escape (value)+"\">"+html_escape (text)+"</button>\n";
}
/*}}}*/

string html_document::make_list (const list<string> &entries, bool numbered)/*{{{*/
{
	string r;

	string list_tag=numbered?"ol":"ul";

	r+="<"+list_tag+">\n";
	
	list<string>::const_iterator end=entries.end ();
	for (list<string>::const_iterator it=entries.begin (); it!=end; ++it)
		if ((*it).find ('\n')==string::npos)
			r+="  <li>"+*it+"</li>\n";	// Single line
		else
			r+="  <li>\n"+*it+"\n</li>\n";	// TODO fix indenting

	r+="</"+list_tag+">\n";

	return r;
}
/*}}}*/

string html_document::make_text_list (const list<string> &entries, bool numbered)/*{{{*/
{
	return make_list (html_escape (entries), numbered);
}
/*}}}*/




html_document &html_document::write_input_text (const string &name, const string &value, unsigned int size)/*{{{*/
{
	write (make_input_text (name, value, size));
	return *this;
}
/*}}}*/

html_document &html_document::write_input_file (const string &name)/*{{{*/
{
	write (make_input_file (name));
	return *this;
}
/*}}}*/

html_document &html_document::write_input_password (const string &name, const string &value)/*{{{*/
{
	write (make_input_password (name, value));
	return *this;
}
/*}}}*/

html_document &html_document::write_input_checkbox (const string &name, const string &text, bool checked)/*{{{*/
{
	write (make_input_checkbox (name, text, checked));
	return *this;
}
/*}}}*/

html_document &html_document::write_input_radio (const string &name, const string &value, const string &text, bool checked)/*{{{*/
{
	write (make_input_radio (name, value, text, checked));
	return *this;
}
/*}}}*/

html_document &html_document::write_submit (const string &caption)/*{{{*/
{
	write (make_submit (caption));
	return *this;
}
/*}}}*/

html_document &html_document::write_input_select (const string &name, const argument_list &options, const string &selected)/*{{{*/
{
	write (make_input_select (name, options, selected));
	return *this;
}
/*}}}*/

html_document &html_document::write_input_select_bool (const string &name, bool is_selected)/*{{{*/
{
	write (make_input_select_bool (name, is_selected));
	return *this;
}
/*}}}*/

html_document &html_document::write_text_button (const string &name, const string &value, const string &text)/*{{{*/
{
	return write (make_text_button (name, value, text));
}
/*}}}*/

html_document &html_document::write_list (const list<string> &entries, bool numbered)/*{{{*/
{
	write (make_list (entries, numbered));
	return *this;
}
/*}}}*/

html_document &html_document::write_text_list (const list<string> &entries, bool numbered)/*{{{*/
{
	write (make_text_list (entries, numbered));
	return *this;
}
/*}}}*/

html_document &html_document::write_preformatted (const string &text)/*{{{*/
{
	start_tag ("pre");
	out << html_escape (text, false);
	end_tag ("pre");

	return *this;
}
/*}}}*/



