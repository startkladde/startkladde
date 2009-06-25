#ifndef table_h
#define table_h

/*
 * table
 * martin
 * 2005-01-29
 */

#include <QString>
#include <list>
#include <vector>


// Table with CSV functionality

namespace csv/*{{{*/
{
	const char delimiter=',';
	const char quote='"';
	const QString newline="\n";
}
/*}}}*/

class table_cell:public QString/*{{{*/
{
	public:
		table_cell (const QString &s):QString (s) {}
		QString csv (bool quote) const;
};
/*}}}*/

class table_row:public std::vector<table_cell>
{
	public:
		static table_row from_csv (const QString &line);
		static table_row from_csv (std::istream &stream);
		QString csv (bool quote) const;
		static void test (const QString &line);
		static void test ();
};

//namespace std
//{
//	ostream &operator<< (ostream &o, const table_row &row);
//}

class table:public std::list<table_row>/*{{{*/
{
	public:
		// Construction
		void read_csv (std::istream &s);
		void read_csv (const QString &text);
		QString csv (bool quote) const;
		static table from_csv (std::istream &s);
		static table from_csv (const QString &text);

		// Testing
		static void test ();
};

//std::ostream &operator<< (std::ostream &o, const table &tab);
/*}}}*/

#endif

