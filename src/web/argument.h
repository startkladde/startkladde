#ifndef argument_h
#define argument_h

/*
 * argument
 * martin
 * 2004-12-29
 */

#include <iostream>
#include <QString>
#include <QList>

#include "src/accessor.h"

// TODO split in two files

class argument
{
	public:
		// Construction
		argument (const QString &_name="", const QString &_value="")
			:name (_name), value (_value) {}
		static argument from_cgi_argument (const QString &query_argument);

		// Reading
		RO_ACCESSOR (QString, name)
		RO_ACCESSOR (QString, value)
		QString make_cgi_parameter () const;

		// Read/Write to file
		std::ostream &write_to (std::ostream &s) const;
		static argument read_from (const QString &s);

	private:
		QString name;
		QString value;

		friend class argument_list;
		friend std::ostream &operator<< (std::ostream &s, const argument &arg);
};

std::ostream &operator<< (std::ostream &s, const argument &arg);

class argument_list
{
	public:
		typedef QList<argument>::iterator iterator;
		typedef QList<argument>::const_iterator const_iterator;
		iterator begin () { return args.begin (); }
		iterator end () { return args.end (); }
		const_iterator begin () const { return args.begin (); }
		const_iterator end () const { return args.end (); }

		// Construction
		argument_list () {};
		static argument_list from_cgi_query (const QString &query_string);

		// Writing
		void set_value (const QString &name, const QString &value="");
		void set_value (const argument &arg);
		void remove (const QString &name);
		void clear ();
		void add (const argument_list &other);

		// Reading
		bool has_argument (const QString &name) const;
		QString get_value (const QString &name) const;
		argument get_argument (const QString &name) const;
		const QList<argument> &get_list () const { return args; }
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
		QList<argument> args;

		// Finding
		iterator find_by_name (const QString &name);
		const_iterator find_by_name (const QString &name) const;

	friend std::ostream &operator<< (std::ostream &s, const argument_list &argl);
};
std::ostream &operator<< (std::ostream &s, const argument_list &argl);

#endif

