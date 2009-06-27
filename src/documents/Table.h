#ifndef _Table_h
#define _Table_h

#include <QString>
#include <QList>


// Table with CSV functionality

namespace csv
{
	const char delimiter=',';
	const char quote='"';
	const QString newline="\n";
}

class TableCell:public QString
{
	public:
		TableCell (const QString &s):QString (s) {}
		QString csv (bool quote) const;
};

class TableRow:public QList<TableCell>
{
	public:
		static TableRow from_csv (const QString &line);
		static TableRow from_csv (std::istream &stream);
		QString csv (bool quote) const;
		static void test (const QString &line);
		static void test ();
};

//namespace std
//{
//	ostream &operator<< (ostream &o, const TableRow &row);
//}

class Table:public QList<TableRow>
{
	public:
		// Construction
		void read_csv (std::istream &s);
		void read_csv (const QString &text);
		QString csv (bool quote) const;
		static Table from_csv (std::istream &s);
		static Table from_csv (const QString &text);

		// Testing
		static void test ();
};

//std::ostream &operator<< (std::ostream &o, const Table &tab);

#endif

