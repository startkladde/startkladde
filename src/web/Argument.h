#ifndef _Argument_h
#define _Argument_h

#include <iostream>
#include <QString>
#include <QList>

#include "src/accessor.h"

// TODO split in two files

class Argument
{
	public:
		// Construction
		Argument (const QString &_name="", const QString &_value="")
			:name (_name), value (_value) {}
		static Argument from_cgi_argument (const QString &query_argument);

		// Reading
		RO_ACCESSOR (QString, name)
		RO_ACCESSOR (QString, value)
		QString make_cgi_parameter () const;

		// Read/Write to file
		std::ostream &write_to (std::ostream &s) const;
		static Argument read_from (const QString &s);

	private:
		QString name;
		QString value;

		friend class ArgumentList;
		friend std::ostream &operator<< (std::ostream &s, const Argument &arg);
};

std::ostream &operator<< (std::ostream &s, const Argument &arg);

class ArgumentList
{
	public:
		typedef QList<Argument>::iterator iterator;
		typedef QList<Argument>::const_iterator const_iterator;
		iterator begin () { return args.begin (); }
		iterator end () { return args.end (); }
		const_iterator begin () const { return args.begin (); }
		const_iterator end () const { return args.end (); }

		// Construction
		ArgumentList () {};
		static ArgumentList from_cgi_query (const QString &query_string);

		// Writing
		void set_value (const QString &name, const QString &value="");
		void set_value (const Argument &arg);
		void remove (const QString &name);
		void clear ();
		void add (const ArgumentList &other);

		// Reading
		bool has_argument (const QString &name) const;
		QString get_value (const QString &name) const;
		Argument get_argument (const QString &name) const;
		const QList<Argument> &get_list () const { return args; }
		int size () const { return args.size (); }
		QString make_cgi_parameters () const;
		bool empty () const { return args.empty (); }

		// Read/Write to file
		std::ostream &write_to (std::ostream &s) const;
		bool write_to_file (const QString &file_name) const;
		void read_from (std::istream &s);
		bool read_from_file (const QString &file_name);

		// Test functions
		static void test ();

	private:
		QList<Argument> args;

		// Finding
		iterator find_by_name (const QString &name);
		const_iterator find_by_name (const QString &name) const;

	friend std::ostream &operator<< (std::ostream &s, const ArgumentList &argl);
};
std::ostream &operator<< (std::ostream &s, const ArgumentList &argl);

#endif

