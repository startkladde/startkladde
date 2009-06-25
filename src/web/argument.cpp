#include "argument.h"

#include <fstream>

#include <errno.h>

#include "src/text.h"


// ***** Argument
// Construction
argument argument::from_cgi_argument (const QString &query_argument)
	/*
	 * Constructs an argument from a www-form-url-encoded query QString part
	 * used in CGI queries (with unescaping).
	 * Example: "name=value"
	 */
{
	argument r;

	QStringList parts=query_argument.split ('=');

	if (parts.empty ())
	{
		std::cerr << "Error: \"" << query_argument << "\" contains no parts" << std::endl;
	}
	else if (parts.size ()==1)
	{
		std::cerr << "Warning: \"" << query_argument << "\" contains less than 2 parts" << std::endl;
		r.name=cgi_unescape (parts.front ());
	}
	else
	{
		if (parts.size ()>2)
			std::cerr << "Warning: \"" << query_argument << "\" contains more than 2 parts" << std::endl;

		r.name=cgi_unescape (*(parts.begin ()));
		r.value=cgi_unescape (*(++parts.begin ()));
	}

	return r;
}


// Reading
QString argument::make_cgi_parameter () const
{
	return cgi_escape (name)+"="+cgi_escape (value);
}


// I/O
std::ostream &operator<< (std::ostream &s, const argument &arg)
	/*
	 * Writes an argument to a stream.
	 */
{
	s << arg.name << "\t" << arg.value << std::endl;
	return s;
}

std::ostream &argument::write_to (std::ostream &s) const
{
	s << cgi_escape (name, true) << "=" << cgi_escape (value, true) << std::endl;
	return s;
}

argument argument::read_from (const QString &s)
{
	return from_cgi_argument (s);
}


// ***** Argument list

// Construction
argument_list argument_list::from_cgi_query (const QString &query_string)
	/*
	 * Constructs an argument list from a www-form-url-encoded query QString
	 * used in CGI queries.
	 * Example: "name1=value1&name2=value2"
	 */
{
	argument_list r;

	QStringList arg_strings=query_string.split ('&');

	QStringListIterator it (arg_strings);
	while (it.hasNext ())
		r.set_value (argument::from_cgi_argument (it.next ()));

	return r;
}


// Writing
void argument_list::set_value (const QString &name, const QString &value)
	/*
	 * If there is an entry with name 'name', it is set to 'value'. If not, a
	 * new one is created.
	 */
{
	std::list<argument>::iterator it=find_by_name (name);
	if (it!=args.end ())
		(*it).value=value;
	else
		args.push_back (argument (name, value));
}

void argument_list::set_value (const argument &arg)
	/*
	 * If there is an entry with name 'arg.name', it is set to 'arg.value'. If
	 * not, a new one is created.
	 */
{
	set_value (arg.name, arg.value);
}

void argument_list::remove (const QString &name)
{
	std::list<argument>::const_iterator end=args.end ();
	for (std::list<argument>::iterator it=args.begin (); it!=end; ++it)
	{
		if (name==(*it).get_name ())
		{
			it=args.erase (it);
			end=args.end ();
		}
	}
}

void argument_list::clear ()
{
	args.clear ();
}

void argument_list::add (const argument_list &other)
{
	const std::list<argument> &other_list=other.get_list ();
	std::list<argument>::const_iterator end=other_list.end ();
	for (std::list<argument>::const_iterator it=other_list.begin (); it!=end; ++it)
		set_value (*it);
}


// Reading
bool argument_list::has_argument (const QString &name) const
	/*
	 * Returns whether there is an entry with name 'name'.
	 */
{
	if (find_by_name (name)==args.end ())
		return false;
	else
		return true;
}

QString argument_list::get_value (const QString &name) const
	/*
	 * If there is an entry with name 'name', its value is returned. If not, an
	 * empty QString is returned.
	 */
{
	std::list<argument>::const_iterator it=find_by_name (name);
	if (it==args.end ())
		return "";
	else
		return (*it).value;
}

argument argument_list::get_argument (const QString &name) const
	/*
	 * If there is an entry with name 'name', it is returned. If not, an
	 * empty argument is returned.
	 */
{
	std::list<argument>::const_iterator it=find_by_name (name);
	if (it==args.end ())
		return argument ();
	else
		return (*it);
}

QString argument_list::make_cgi_parameters () const
	/*
	 * Returns a CGI escaped QString of the parameter list.
	 * Example: foo=bar&baz=q+u+x
	 */
{
	QString r;
	std::list<argument>::const_iterator end=args.end ();
	for (std::list<argument>::const_iterator it=args.begin (); it!=end; ++it)
	{
		if (it!=args.begin ()) r+="&";
		r+=(*it).make_cgi_parameter ();
	}

	return r;
}


// I/O
std::ostream &operator<< (std::ostream &s, const argument_list &argl)
	/*
	 * Writes the list to a stream.
	 */
{
	std::list<argument>::const_iterator e=argl.args.end ();
	for (std::list<argument>::const_iterator it=argl.args.begin (); it!=e; ++it)
		s << *it;

	return s;
}

std::ostream &argument_list::write_to (std::ostream &s) const
{
	std::list<argument>::const_iterator e=args.end ();
	for (std::list<argument>::const_iterator it=args.begin (); it!=e; ++it)
		(*it).write_to (s);

	return s;
}

bool argument_list::write_to_file (const QString &file_name) const
{
	// TODO Qt-ize
	std::ofstream file (file_name.latin1());
	if (!file)
	{
		return false;
	}

	write_to (file);
	file.close ();
	return true;
}

// TODO Qt-ize
void argument_list::read_from (std::istream &s)
{
	std::string line;
	while (s.good ())
	{
		getline (s, line);
		if (!line.empty ()) set_value (argument::read_from (std2q (line)));
	}
}

bool argument_list::read_from_file (const QString &file_name)
{
	// TODO Qt-ize
	std::ifstream file (file_name.latin1 ());
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


// Finding
std::list<argument>::iterator argument_list::find_by_name (const QString &name)
	/*
	 * If there is an entry with name 'name', an iterator to it is returned. If
	 * not, an iterator to the end of the list is returned.
	 */
{
	std::list<argument>::iterator e=args.end ();
	for (std::list<argument>::iterator it=args.begin (); it!=e; ++it)
		if (name==(*it).get_name ())
			return it;
	return e;
}

std::list<argument>::const_iterator argument_list::find_by_name (const QString &name) const
	/*
	 * If there is an entry with name 'name', an iterator to it is returned. If
	 * not, an iterator to the end of the list is returned.
	 */
{
	std::list<argument>::const_iterator e=args.end ();
	for (std::list<argument>::const_iterator it=args.begin (); it!=e; ++it)
		if (name==(*it).get_name ())
			return it;
	return e;
}


// Test functions
void argument_list::test ()
{
	argument_list argl;
	argl.set_value ("foo", "b a r");
	argl.set_value ("m�ller", "I\nam\nnot\nwithout\nnewlines!");

	argl.set_value ("hello", "world");
	std::cout << argl << std::endl;
	argl.remove ("hello");

	std::cout << argl << std::endl;
	if (argl.write_to_file ("/tmp/test.arglist"))
		std::cout << "Write success" << std::endl;
	else
		std::cout << "Write failure" << std::endl;

	argl.clear ();
	argl.read_from_file ("/tmp/test.arglist");
	std::cout << std::endl;
	std::cout << argl;
}







