#ifndef argument_h
#define argument_h

/*
 * argument
 * martin
 * 2004-12-29
 */

#include <iostream>
#include <list>
#include <string>

#include "src/accessor.h"

// TODO split in two files

using namespace std;

class argument
{
	public:
		// Construction
		argument (const string &_name="", const string &_value="")
			:name (_name), value (_value) {}
		static argument from_cgi_argument (const string &query_argument);

		// Reading
		RO_ACCESSOR (string, name)
		RO_ACCESSOR (string, value)
		string make_cgi_parameter () const;

		// Read/Write to file
		ostream &write_to (ostream &s) const;
		static argument read_from (const string &s);

	private:
		string name;
		string value;

		friend class argument_list;
		friend ostream &operator<< (ostream &s, const argument &arg);
};
ostream &operator<< (ostream &s, const argument &arg);

class argument_list
{
	public:
		typedef list<argument>::iterator iterator;
		typedef list<argument>::const_iterator const_iterator;
		iterator begin () { return args.begin (); }
		iterator end () { return args.end (); }
		const_iterator begin () const { return args.begin (); }
		const_iterator end () const { return args.end (); }

		// Construction
		argument_list () {};
		static argument_list from_cgi_query (const string &query_string);

		// Writing
		void set_value (const string &name, const string &value="");
		void set_value (const argument &arg);
		void remove (const string &name);
		void clear ();
		void add (const argument_list &other);

		// Reading
		bool has_argument (const string &name) const;
		string get_value (const string &name) const;
		argument get_argument (const string &name) const;
		const list<argument> &get_list () const { return args; }
		int size () const { return args.size (); }
		string make_cgi_parameters () const;
		bool empty () const { return args.empty (); }

		// Read/Write to file
		ostream &write_to (ostream &s) const;
		bool write_to_file (const string &file_name) const;
		void read_from (istream &s);
		bool read_from_file (const string &file_name);

		// Test functions
		static void test ();

	private:
		list<argument> args;

		// Finding
		iterator find_by_name (const string &name);
		const_iterator find_by_name (const string &name) const;

	friend ostream &operator<< (ostream &s, const argument_list &argl);
};
ostream &operator<< (ostream &s, const argument_list &argl);

#endif

