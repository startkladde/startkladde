#include "Table.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "src/text.h"

enum csv_parser_state { ps_first_data, ps_data, ps_quoted_data, ps_quoted_quote, ps_end };

/**
  * @param quote:  if true, the csv value will be quoted
  * @return the csv value
  */
QString TableCell::csv (bool quote) const
{
	QString r;
	if (quote)
		r.append (csv::quote);

	const_iterator e=end ();
	for (const_iterator it=begin (); it!=e; ++it)
	{
		if ((*it)==csv::quote)
		{
			r.append (csv::quote);
			r.append (csv::quote);
		}
		else
		{
			r.append (*it);
		}
	}

	if (quote)
		r.append (csv::quote);

	return r;
}


TableRow TableRow::from_csv (const QString &line)
{
	TableRow row;
	QString current_cell;

#define APPEND_CELL do { row.push_back (TableCell (current_cell)); current_cell.clear (); } while (false)
#define APPEND_CHAR do { current_cell.push_back (ch); } while (false)

#define DO(ACTION) do { ACTION; } while (false)
#define GO(STATE) do { state=STATE; } while (false)
#define DO_GO(ACTION, STATE) do { DO (ACTION); GO (STATE); } while (false)

	QString::const_iterator begin=line.begin ();
	QString::const_iterator end=line.end ();

	while ((*end==0x0D || *end==0x0A) && end!=begin) --end;

	QString::const_iterator it=begin;
	csv_parser_state state=ps_first_data;
	while (state!=ps_end)
	{
		if (it==end)
		{

			state=ps_end;
		}
		else
		{
			QChar ch=*it;

			switch (state)
			{
				case ps_first_data:
					if (ch==csv::delimiter)
						DO (APPEND_CELL);
					else if (ch==csv::quote)
						GO (ps_quoted_data);
					else
						DO_GO (APPEND_CHAR, ps_data);
					break;
				case ps_data:
					if (ch==csv::delimiter)
						DO_GO (APPEND_CELL, ps_first_data);
					else
						DO (APPEND_CHAR);
					break;
				case ps_quoted_data:
					if (ch==csv::quote)
						GO (ps_quoted_quote);
					else
						DO (APPEND_CHAR);
					break;
				case ps_quoted_quote:
					if (ch==csv::quote)
						DO_GO (APPEND_CHAR, ps_quoted_data);
					else if (ch==csv::delimiter)
						DO_GO (APPEND_CELL, ps_first_data);
					else
						{} // Error, ignored.
					break;
				case ps_end:
					// Nothing
					break;
			}
		}
		++it;
	}

	row.push_back (TableCell (current_cell));

	return row;
}

TableRow TableRow::from_csv (std::istream &stream)
{
	// TODO Qt-ize
	std::string line;
	if (stream.eof ())
	{
		return TableRow ();
	}
	else
	{
		getline (stream, line);
		return from_csv (std2q (line));
	}
}

/**
  * @param quote:  if true, the csv values will be quoted
  * @return the csv values
  */
QString TableRow::csv (bool quote) const
{
	QString r;

	const_iterator e=end ();
	for (const_iterator it=begin (); it!=e; ++it)
	{
		if (it!=begin ()) r.append (csv::delimiter);
		r.append ((*it).csv (quote));
	}

	return r;
}

std::ostream &operator<< (std::ostream &o, const TableRow &row)
{
	TableRow::const_iterator end=row.end ();
	for (TableRow::const_iterator it=row.begin (); it!=end; ++it)
	{
		o << "[" << *it << "]";
	}
	o << std::endl;

	return o;
}

void TableRow::test (const QString &text)
{
	TableRow row=TableRow::from_csv (text);
	std::cout << row;
}

void TableRow::test ()
{
	test ("ab,\"cd,ef\",gh");
	test ("ij,\"k\"\"l\",\"m,n\"");
}




void Table::read_csv (std::istream &s)
{
	// TODO Qt-ize
	std::string line;
	while (!s.eof ())
	{
		getline (s, line);
		if (!line.empty ())
			push_back (TableRow::from_csv (std2q (line)));
	}
}

void Table::read_csv (const QString &s)
{
	// TODO Qt-ize
	std::istringstream stream (q2std (s));
	read_csv (stream);
}

Table Table::from_csv (std::istream &s)
{
	Table t;
	t.read_csv (s);
	return t;
}

Table Table::from_csv (const QString &s)
{
	Table t;
	t.read_csv (s);
	return t;
}

/**
  * @param quote:  if true, the csv values will be quoted
  * @return the csv values
  */
QString Table::csv (bool quote) const
{
	QString r;

	const_iterator e=end ();
	for (const_iterator it=begin (); it!=e; ++it)
	{
		// No newline at the end. What was this good for?
//		if (it!=begin ()) r.append (csv::newline);
		r.append ((*it).csv (quote));
		r.append (csv::newline);
	}

	return r;
}

std::ostream &operator<< (std::ostream &o, const Table &tab)
{
	Table::const_iterator end=tab.end ();
	for (Table::const_iterator it=tab.begin (); it!=end; ++it)
	{
		o << *it;
	}

	return o;
}

void Table::test ()
{
	Table t=Table::from_csv ("foo,\"bar,\"\"baz\"\"\"\nqux,qu\"ux\nquuux,\n,gaga");
	std::cout << t;


}


