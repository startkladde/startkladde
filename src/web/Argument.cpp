#include "Argument.h"

#include <fstream>

#include <errno.h>

#include "src/text.h"


// ***** Argument
// Construction
Argument Argument::from_cgi_argument (const QString &query_argument)
	/*
	 * Constructs an Argument from a www-form-url-encoded query QString part
	 * used in CGI queries (with unescaping).
	 * Example: "name=value"
	 */
{
	Argument r;

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
QString Argument::make_cgi_parameter () const
{
	return cgi_escape (name)+"="+cgi_escape (value);
}


// I/O
std::ostream &operator<< (std::ostream &s, const Argument &arg)
	/*
	 * Writes an Argument to a stream.
	 */
{
	s << arg.name << "\t" << arg.value << std::endl;
	return s;
}

std::ostream &Argument::write_to (std::ostream &s) const
{
	s << cgi_escape (name, true) << "=" << cgi_escape (value, true) << std::endl;
	return s;
}

Argument Argument::read_from (const QString &s)
{
	return from_cgi_argument (s);
}


// ***** Argument list

// Construction
ArgumentList ArgumentList::from_cgi_query (const QString &query_string)
	/*
	 * Constructs an Argument list from a www-form-url-encoded query QString
	 * used in CGI queries.
	 * Example: "name1=value1&name2=value2"
	 */
{
	ArgumentList r;

	QStringList arg_strings=query_string.split ('&');

	QStringListIterator it (arg_strings);
	while (it.hasNext ())
		r.set_value (Argument::from_cgi_argument (it.next ()));

	return r;
}


// Writing
void ArgumentList::set_value (const QString &name, const QString &value)
	/*
	 * If there is an entry with name 'name', it is set to 'value'. If not, a
	 * new one is created.
	 */
{
	QList<Argument>::iterator it=find_by_name (name);
	if (it!=args.end ())
		(*it).value=value;
	else
		args.push_back (Argument (name, value));
}

void ArgumentList::set_value (const Argument &arg)
	/*
	 * If there is an entry with name 'arg.name', it is set to 'arg.value'. If
	 * not, a new one is created.
	 */
{
	set_value (arg.name, arg.value);
}

void ArgumentList::remove (const QString &name)
{
	QList<Argument>::const_iterator end=args.end ();
	for (QList<Argument>::iterator it=args.begin (); it!=end; ++it)
	{
		if (name==(*it).get_name ())
		{
			it=args.erase (it);
			end=args.end ();
		}
	}
}

void ArgumentList::clear ()
{
	args.clear ();
}

void ArgumentList::add (const ArgumentList &other)
{
	const QList<Argument> &other_list=other.get_list ();
	QList<Argument>::const_iterator end=other_list.end ();
	for (QList<Argument>::const_iterator it=other_list.begin (); it!=end; ++it)
		set_value (*it);
}


// Reading
bool ArgumentList::has_argument (const QString &name) const
	/*
	 * Returns whether there is an entry with name 'name'.
	 */
{
	if (find_by_name (name)==args.end ())
		return false;
	else
		return true;
}

QString ArgumentList::get_value (const QString &name) const
	/*
	 * If there is an entry with name 'name', its value is returned. If not, an
	 * empty QString is returned.
	 */
{
	QList<Argument>::const_iterator it=find_by_name (name);
	if (it==args.end ())
		return "";
	else
		return (*it).value;
}

Argument ArgumentList::get_argument (const QString &name) const
	/*
	 * If there is an entry with name 'name', it is returned. If not, an
	 * empty Argument is returned.
	 */
{
	QList<Argument>::const_iterator it=find_by_name (name);
	if (it==args.end ())
		return Argument ();
	else
		return (*it);
}

QString ArgumentList::make_cgi_parameters () const
	/*
	 * Returns a CGI escaped QString of the parameter list.
	 * Example: foo=bar&baz=q+u+x
	 */
{
	QString r;
	QList<Argument>::const_iterator end=args.end ();
	for (QList<Argument>::const_iterator it=args.begin (); it!=end; ++it)
	{
		if (it!=args.begin ()) r+="&";
		r+=(*it).make_cgi_parameter ();
	}

	return r;
}


// I/O
std::ostream &operator<< (std::ostream &s, const ArgumentList &argl)
	/*
	 * Writes the list to a stream.
	 */
{
	QList<Argument>::const_iterator e=argl.args.end ();
	for (QList<Argument>::const_iterator it=argl.args.begin (); it!=e; ++it)
		s << *it;

	return s;
}

std::ostream &ArgumentList::write_to (std::ostream &s) const
{
	QList<Argument>::const_iterator e=args.end ();
	for (QList<Argument>::const_iterator it=args.begin (); it!=e; ++it)
		(*it).write_to (s);

	return s;
}

bool ArgumentList::write_to_file (const QString &file_name) const
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
void ArgumentList::read_from (std::istream &s)
{
	std::string line;
	while (s.good ())
	{
		getline (s, line);
		if (!line.empty ()) set_value (Argument::read_from (std2q (line)));
	}
}

bool ArgumentList::read_from_file (const QString &file_name)
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
QList<Argument>::iterator ArgumentList::find_by_name (const QString &name)
	/*
	 * If there is an entry with name 'name', an iterator to it is returned. If
	 * not, an iterator to the end of the list is returned.
	 */
{
	QList<Argument>::iterator e=args.end ();
	for (QList<Argument>::iterator it=args.begin (); it!=e; ++it)
		if (name==(*it).get_name ())
			return it;
	return e;
}

QList<Argument>::const_iterator ArgumentList::find_by_name (const QString &name) const
	/*
	 * If there is an entry with name 'name', an iterator to it is returned. If
	 * not, an iterator to the end of the list is returned.
	 */
{
	QList<Argument>::const_iterator e=args.end ();
	for (QList<Argument>::const_iterator it=args.begin (); it!=e; ++it)
		if (name==(*it).get_name ())
			return it;
	return e;
}


// Test functions
void ArgumentList::test ()
{
	ArgumentList argl;
	argl.set_value ("foo", "b a r");
	argl.set_value ("müller", "I\nam\nnot\nwithout\nnewlines!");

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







