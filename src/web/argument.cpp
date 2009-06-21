#include "argument.h"

#include <fstream>

#include <errno.h>

#include "src/text.h"


// ***** Argument
// Construction
argument argument::from_cgi_argument (const string &query_argument)/*{{{*/
	/*
	 * Constructs an argument from a www-form-url-encoded query string part
	 * used in CGI queries (with unescaping).
	 * Example: "name=value"
	 */
{
	argument r;

	list<string> parts;
	split_string (parts, "=", query_argument);

	if (parts.empty ())
	{
		cerr << "Error: \"" << query_argument << "\" contains no parts" << endl;
	}
	else if (parts.size ()==1)
	{
		cerr << "Warning: \"" << query_argument << "\" contains less than 2 parts" << endl;
		r.name=cgi_unescape (parts.front ());
	}
	else
	{
		if (parts.size ()>2)
			cerr << "Warning: \"" << query_argument << "\" contains more than 2 parts" << endl;

		r.name=cgi_unescape (*(parts.begin ()));
		r.value=cgi_unescape (*(++parts.begin ()));
	}

	return r;
}
/*}}}*/


// Reading
string argument::make_cgi_parameter () const/*{{{*/
{
	return cgi_escape (name)+"="+cgi_escape (value);
}
/*}}}*/


// I/O
ostream &operator<< (ostream &s, const argument &arg)/*{{{*/
	/*
	 * Writes an argument to a stream.
	 */
{
	s << arg.name << "\t" << arg.value << endl;
	return s;
}
/*}}}*/

ostream &argument::write_to (ostream &s) const/*{{{*/
{
	s << cgi_escape (name, true) << "=" << cgi_escape (value, true) << endl;
	return s;
}
/*}}}*/

argument argument::read_from (const string &s)/*{{{*/
{
	return from_cgi_argument (s);
}
/*}}}*/


// ***** Argument list

// Construction
argument_list argument_list::from_cgi_query (const string &query_string)/*{{{*/
	/*
	 * Constructs an argument list from a www-form-url-encoded query string
	 * used in CGI queries.
	 * Example: "name1=value1&name2=value2"
	 */
{
	argument_list r;

	list<string> arg_strings;
	split_string (arg_strings, "&", query_string);

	list<string>::const_iterator e=arg_strings.end ();
	for (list<string>::const_iterator it=arg_strings.begin (); it!=e; ++it)
		r.set_value (argument::from_cgi_argument (*it));

	return r;
}
/*}}}*/


// Writing
void argument_list::set_value (const string &name, const string &value)/*{{{*/
	/*
	 * If there is an entry with name 'name', it is set to 'value'. If not, a
	 * new one is created.
	 */
{
	list<argument>::iterator it=find_by_name (name);
	if (it!=args.end ())
		(*it).value=value;
	else
		args.push_back (argument (name, value));
}
/*}}}*/

void argument_list::set_value (const argument &arg)/*{{{*/
	/*
	 * If there is an entry with name 'arg.name', it is set to 'arg.value'. If
	 * not, a new one is created.
	 */
{
	set_value (arg.name, arg.value);
}
/*}}}*/

void argument_list::remove (const string &name)/*{{{*/
{
	list<argument>::const_iterator end=args.end ();
	for (list<argument>::iterator it=args.begin (); it!=end; ++it)
	{
		if (name==(*it).get_name ())
		{
			it=args.erase (it);
			end=args.end ();
		}
	}
}
/*}}}*/

void argument_list::clear ()/*{{{*/
{
	args.clear ();
}
/*}}}*/

void argument_list::add (const argument_list &other)/*{{{*/
{
	const list<argument> &other_list=other.get_list ();
	list<argument>::const_iterator end=other_list.end ();
	for (list<argument>::const_iterator it=other_list.begin (); it!=end; ++it)
		set_value (*it);
}
/*}}}*/


// Reading
bool argument_list::has_argument (const string &name) const/*{{{*/
	/*
	 * Returns whether there is an entry with name 'name'.
	 */
{
	if (find_by_name (name)==args.end ())
		return false;
	else
		return true;
}
/*}}}*/

string argument_list::get_value (const string &name) const/*{{{*/
	/*
	 * If there is an entry with name 'name', its value is returned. If not, an
	 * empty string is returned.
	 */
{
	list<argument>::const_iterator it=find_by_name (name);
	if (it==args.end ())
		return "";
	else
		return (*it).value;
}
/*}}}*/

argument argument_list::get_argument (const string &name) const/*{{{*/
	/*
	 * If there is an entry with name 'name', it is returned. If not, an
	 * empty argument is returned.
	 */
{
	list<argument>::const_iterator it=find_by_name (name);
	if (it==args.end ())
		return argument ();
	else
		return (*it);
}
/*}}}*/

string argument_list::make_cgi_parameters () const/*{{{*/
	/*
	 * Returns a CGI escaped string of the parameter list.
	 * Example: foo=bar&baz=q+u+x
	 */
{
	string r;
	list<argument>::const_iterator end=args.end ();
	for (list<argument>::const_iterator it=args.begin (); it!=end; ++it)
	{
		if (it!=args.begin ()) r+="&";
		r+=(*it).make_cgi_parameter ();
	}

	return r;
}
/*}}}*/


// I/O
ostream &operator<< (ostream &s, const argument_list &argl)/*{{{*/
	/*
	 * Writes the list to a stream.
	 */
{
	list<argument>::const_iterator e=argl.args.end ();
	for (list<argument>::const_iterator it=argl.args.begin (); it!=e; ++it)
		s << *it;

	return s;
}
/*}}}*/

ostream &argument_list::write_to (ostream &s) const/*{{{*/
{
	list<argument>::const_iterator e=args.end ();
	for (list<argument>::const_iterator it=args.begin (); it!=e; ++it)
		(*it).write_to (s);

	return s;
}
/*}}}*/

bool argument_list::write_to_file (const string &file_name) const/*{{{*/
{
	ofstream file (file_name.c_str ());
	if (!file)
	{
		return false;
	}

	write_to (file);
	file.close ();
	return true;
}
/*}}}*/

void argument_list::read_from (istream &s)/*{{{*/
{
	string line;
	while (s.good ())
	{
		getline (s, line);
		if (!line.empty ()) set_value (argument::read_from (line));
	}
}
/*}}}*/

bool argument_list::read_from_file (const string &file_name)/*{{{*/
{
	ifstream file (file_name.c_str ());
	if (file)
	{
		read_from (file);
		file.close ();
		return true;
	}
	else
	{
		return false;
	}
}
/*}}}*/


// Finding
list<argument>::iterator argument_list::find_by_name (const string &name)/*{{{*/
	/*
	 * If there is an entry with name 'name', an iterator to it is returned. If
	 * not, an iterator to the end of the list is returned.
	 */
{
	list<argument>::iterator e=args.end ();
	for (list<argument>::iterator it=args.begin (); it!=e; ++it)
		if (name==(*it).get_name ())
			return it;
	return e;
}
/*}}}*/

list<argument>::const_iterator argument_list::find_by_name (const string &name) const/*{{{*/
	/*
	 * If there is an entry with name 'name', an iterator to it is returned. If
	 * not, an iterator to the end of the list is returned.
	 */
{
	list<argument>::const_iterator e=args.end ();
	for (list<argument>::const_iterator it=args.begin (); it!=e; ++it)
		if (name==(*it).get_name ())
			return it;
	return e;
}
/*}}}*/


// Test functions
void argument_list::test ()/*{{{*/
{
	argument_list argl;
	argl.set_value ("foo", "b a r");
	argl.set_value ("müller", "I\nam\nnot\nwithout\nnewlines!");

	argl.set_value ("hello", "world");
	cout << argl << endl;
	argl.remove ("hello");

	cout << argl << endl;
	if (argl.write_to_file ("/tmp/test.arglist"))
		cout << "Write success" << endl;
	else
		cout << "Write failure" << endl;

	argl.clear ();
	argl.read_from_file ("/tmp/test.arglist");
	cout << endl;
	cout << argl;
}
/*}}}*/







