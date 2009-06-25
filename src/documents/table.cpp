#include "table.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "src/text.h"

enum csv_parser_state { ps_first_data, ps_data, ps_quoted_data, ps_quoted_quote, ps_end };

/**
  * @param quote:  if true, the csv value will be quoted
  * @return the csv value
  */
QString table_cell::csv (bool quote) const
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


table_row table_row::from_csv (const QString &line)/*{{{*/
{
	table_row row;
	QString current_cell;

#define APPEND_CELL do { row.push_back (table_cell (current_cell)); current_cell.clear (); } while (false)
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

	row.push_back (table_cell (current_cell));

	return row;
}
/*}}}*/

table_row table_row::from_csv (std::istream &stream)/*{{{*/
{
	// TODO Qt-ize
	std::string line;
	if (stream.eof ())
	{
		return table_row ();
	}
	else
	{
		getline (stream, line);
		return from_csv (std2q (line));
	}
}
/*}}}*/

/**
  * @param quote:  if true, the csv values will be quoted
  * @return the csv values
  */
QString table_row::csv (bool quote) const
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

std::ostream &operator<< (std::ostream &o, const table_row &row)/*{{{*/
{
	table_row::const_iterator end=row.end ();
	for (table_row::const_iterator it=row.begin (); it!=end; ++it)
	{
		o << "[" << *it << "]";
	}
	o << std::endl;

	return o;
}
/*}}}*/

void table_row::test (const QString &text)/*{{{*/
{
	table_row row=table_row::from_csv (text);
	std::cout << row;
}
/*}}}*/

void table_row::test ()/*{{{*/
{
	test ("ab,\"cd,ef\",gh");
	test ("ij,\"k\"\"l\",\"m,n\"");
}
/*}}}*/




void table::read_csv (std::istream &s)/*{{{*/
{
	// TODO Qt-ize
	std::string line;
	while (!s.eof ())
	{
		getline (s, line);
		if (!line.empty ())
			push_back (table_row::from_csv (std2q (line)));
	}
}
/*}}}*/

void table::read_csv (const QString &s)/*{{{*/
{
	// TODO Qt-ize
	std::istringstream stream (q2std (s));
	read_csv (stream);
}
/*}}}*/

table table::from_csv (std::istream &s)/*{{{*/
{
	table t;
	t.read_csv (s);
	return t;
}
/*}}}*/

table table::from_csv (const QString &s)/*{{{*/
{
	table t;
	t.read_csv (s);
	return t;
}
/*}}}*/

/**
  * @param quote:  if true, the csv values will be quoted
  * @return the csv values
  */
QString table::csv (bool quote) const
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

std::ostream &operator<< (std::ostream &o, const table &tab)/*{{{*/
{
	table::const_iterator end=tab.end ();
	for (table::const_iterator it=tab.begin (); it!=end; ++it)
	{
		o << *it;
	}

	return o;
}
/*}}}*/

void table::test ()/*{{{*/
{
	table t=table::from_csv ("foo,\"bar,\"\"baz\"\"\"\nqux,qu\"ux\nquuux,\n,gaga");
	std::cout << t;


}
/*}}}*/


