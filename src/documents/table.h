#ifndef table_h
#define table_h

/*
 * table
 * martin
 * 2005-01-29
 */

#include <string>
#include <list>
#include <vector>

using namespace std;

// Table with CSV functionality

namespace csv/*{{{*/
{
	const char delimiter=',';
	const char quote='"';
	const string newline="\n";
}
/*}}}*/

class table_cell:public string/*{{{*/
{
	public:
		table_cell (const string &s):string (s) {}	// Why isn't the constructor inherited?
		string csv (bool quote) const;
};
/*}}}*/

class table_row:public vector<table_cell>/*{{{*/
{
	public:
		static table_row from_csv (const string &line);
		static table_row from_csv (istream &stream);
		string csv (bool quote) const;
		static void test (const string &line);
		static void test ();
};

ostream &operator<< (ostream &o, const table_row &row);
/*}}}*/

class table:public list<table_row>/*{{{*/
{
	public:
		// Construction
		void read_csv (istream &s);
		void read_csv (const string &text);
		string csv (bool quote) const;
		static table from_csv (istream &s);
		static table from_csv (const string &text);

		// Testing
		static void test ();
};

ostream &operator<< (ostream &o, const table &tab);
/*}}}*/

#endif

