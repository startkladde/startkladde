#include "text.h"

#include <QStringList>

#include <stdlib.h>

const QString whitespace=" \t\r\n";

bool eintrag_ist_leer (QString eintrag)
	/*
	 * Finds out whether an entry is considered "empty".
	 * Parameters:
	 *   - eintrag: the QString.
	 * Return value:
	 *   if eintrag is to be considered "empty".
	 */
{
	QString e=eintrag.simplified ();
	return (e=="" || e=="-");
}

bool check_message (QWidget *parent, const QString &msg)
	/*
	 * Displays a message and ask the user if he wishes to accept anyway.
	 * Parameters:
	 *   - parent: passed on to the QMessageBox constructor.
	 *   - msg: the message.
	 * Return value:
	 *   - if the user accepted.
	 */
{
	int ret=QMessageBox::information (parent, "Warnung",
			msg+"Trotzdem akzeptieren?", "&Ja", "&Nein", QString::null, 0, 1)
			!=QDialog::Accepted;
	return (ret==QDialog::Accepted);
}

QString t_pilot_bezeichnung (FlightType flugtyp, casus c)
	/*
	 * Creates a description of the pilot's function (pilot, flight instructor...).
	 * Parameters:
	 *   - flugytp: the flight type.
	 *   - casus c: the case (grammatical) to create the description with.
	 * Return value:
	 *   - the description.
	 */
{
	// TODO diese hier in data_types.cpp
	switch (flugtyp)
	{
		case ftTraining2: return c==cas_genitiv   ? QString::fromUtf8 ("Flugschülers") : QString::fromUtf8 ("Flugschüler");
		default:         return c==cas_nominativ ? QString::fromUtf8 ("Pilot")        : QString::fromUtf8 ("Piloten");
	}
}

QString t_begleiter_bezeichnung (FlightType flugtyp, casus c)
	/*
	 * Creates a description of the copilot's function (pilot, flight instructor...).
	 * Parameters:
	 *   - flugytp: the flight type.
	 *   - casus c: the case (grammatical) to create the description with.
	 * Return value:
	 *   - the description.
	 */
{
	switch (flugtyp)
	{
		case ftTraining2:                          return c==cas_genitiv ? "Fluglehrers": "Fluglehrer";
		case ftGuestPrivate: case ftGuestExternal: return c==cas_genitiv ? "Gasts"      : "Gast";
		default:                                  return c==cas_genitiv ? "Begleiters" : "Begleiter";
	}
}

void replace_tabs (QString &s)
{
	s.replace ('\t', ' ');
}

QString simplify_club_name (const QString s)
{
	QString r=s.simplified ().toLower ();
	if (eintrag_ist_leer (r))
		return "-";
	else
		return r;
}

bool club_name_identical (const QString s1, const QString s2)
{
	return simplify_club_name (s1)==simplify_club_name (s2);
}

QString concatenate_comments (const QString& s1, const QString& s2)
{
	if (eintrag_ist_leer (s1)) return s2;
	if (eintrag_ist_leer (s2)) return s1;
	return s1+"; "+s2;
}

QString string_or_none (const QString& text)
{
	if (eintrag_ist_leer (text))
		return "-";
	else
		return text;
}

bool parameter_matches (char *parameter, char *val1, char *val2)
{
	return ((strcmp (parameter, val1)==0) || (strcmp (parameter, val2)==0));
}

#include <iostream>

void split_string (QString &string1, QString &string2, QString separator, QString text)
	// Splits a QString at the first occurence of one of separators. Results in string1 and string2
{
	if (text.contains (separator))
	{
		int pos=text.indexOf (separator);

		string1=text.mid (0, pos);
		string2=text.mid (pos+1);
	}
	else
	{
		string1=text;
		string2="";
	}
}

void trim (QStringList &strings)
{
	QMutableStringListIterator it (strings);
	while (it.hasNext ())
	{
		it.next ();
		it.value()=it.value().trimmed();
	}
}

bool is_date (const QString& s)
	/*
	 * Finds out if a QString can be parsed as date.
	 * Parameters:
	 *   - s: the QString to test.
	 * Return value:
	 *   - true if s is a date.
	 *   - false else.
	 */
{
	// TODO remove as the result is discarded.
	return QDate::fromString (s, Qt::ISODate).isValid ();
}

QString latex_escape (const QString& o)
	/*
	 * Escape a QString for putting to a LaTeX document.
	 * Parameters:
	 *   - o: the QString to escape.
	 * Return value:
	 *   - the escaped QString.
	 */
{
	QString result;

	for (int i=0; i<o.length (); i++)
	{
		if      (o[i]=='$' ) result+="\\$";
		else if (o[i]=='\\') result+="\\textbackslash{}";
		else if (o[i]=='%' ) result+="\\%";
		else if (o[i]=='_' ) result+="\\_";
		else if (o[i]=='{' ) result+="\\{";
		else if (o[i]=='&' ) result+="\\&";
		else if (o[i]=='#' ) result+="\\#";
		else if (o[i]=='}' ) result+="\\}";
		else if (o[i]=='^' ) result+="\\textasciicircum{}";
		else if (o[i]=='~' ) result+="\\textasciitilde{}";
		else if (o[i]=='"' ) result+="\\textquotedbl{}";
		// Seems like guilsingl{left,right} don't work properly
		//case '<': oss << "\\guilsinglleft{}"; break;
		//case '>': oss << "\\guilsinglright{}"; break;
		else result+=o[i];
	}

	return result;
}

/**
  * Escape a QString for putting to a LaTeX document.
  * Parameters:
  *   - o: the QString to escape.
  * Return value:
  *   - the escaped QString.
  */
QString csv_escape (const QString& o)
{
	// TODO werden ""en korrekt behandelt?
	QString result;

	QChar quote_char='"';

	result+=quote_char;
	for (int i=0; i<o.length (); i++)
	{
		if (o[i]==quote_char)
			(result+=quote_char)+=quote_char;
		else
			result+=o[i];
	}
	result+=quote_char;

	return result;
}

QString html_escape (const QString& s, bool also_escape_newlines)
{
	QString r;

	QString::const_iterator end=s.end ();
	for (QString::const_iterator ch=s.begin (); ch!=end; ++ch)
	{
		if (*ch=='<') r+="&lt;";
		else if (*ch=='>') r+="&gt;";
		else if (*ch=='"') r+="&quot;";
		// Don't escape spaces because this will break (hidden) CGI arguments.
//		else if (*ch==' ') r+="&nbsp;";
		else if (*ch=='&') r+="&amp;";
		else if (also_escape_newlines && *ch=='\n') r+="<br>";
		else r+=QString (1, *ch);
	}

	return r;
}

QStringList html_escape (const QStringList &l, bool also_escape_newlines)
{
	QStringList r;
	QStringListIterator it (l);
	while (it.hasNext ())
		r.append (html_escape (it.next(), also_escape_newlines));

	return r;
}

QString get_environment (const QString& name)
{
	char *r=getenv (name.latin1());
	if (r)
		return QString (r);
	else
		return QString ();
}


char hex_digit_value (char digit)
{
	switch (digit)
	{
		case '0': return 0; break;
		case '1': return 1; break;
		case '2': return 2; break;
		case '3': return 3; break;
		case '4': return 4; break;
		case '5': return 5; break;
		case '6': return 6; break;
		case '7': return 7; break;
		case '8': return 8; break;
		case '9': return 9; break;
		case 'a': case 'A': return 10; break;
		case 'b': case 'B': return 11; break;
		case 'c': case 'C': return 12; break;
		case 'd': case 'D': return 13; break;
		case 'e': case 'E': return 14; break;
		case 'f': case 'F': return 15; break;
		default: return 0; break;
	}
}

QString hex_digit (char value)
{
	switch (value%16)
	{
		case  0: return "0"; break;
		case  1: return "1"; break;
		case  2: return "2"; break;
		case  3: return "3"; break;
		case  4: return "4"; break;
		case  5: return "5"; break;
		case  6: return "6"; break;
		case  7: return "7"; break;
		case  8: return "8"; break;
		case  9: return "9"; break;
		case 10: return "A"; break;
		case 11: return "B"; break;
		case 12: return "C"; break;
		case 13: return "D"; break;
		case 14: return "E"; break;
		case 15: return "F"; break;
	}
	return "?";
}

QString hex_string (unsigned char value)
{
	return hex_digit (value/16)+hex_digit (value%16);
}

QString cgi_unescape (const QString &text)
{
	enum state_t { st_normal, st_expect_first, st_expect_second };
	state_t state=st_normal;

	QString r;
	char special;

	QString::const_iterator e=text.end ();
	for (QString::const_iterator it=text.begin (); it!=e; ++it)
	{
		switch (state)
		{
			case st_normal:
				if (*it==' '|| *it=='\n'|| *it=='\t')
					{}	// ignore
				else if (*it=='%')
					state=st_expect_first;
				else if (*it=='+')
					r+=" ";
				else
					r+=QString (1, *it);
				break;
			case st_expect_first:
				special=16*hex_digit_value ((*it).toAscii());
				state=st_expect_second;
				break;
			case st_expect_second:
				special+=hex_digit_value ((*it).toAscii());
				r+=QString (1, special);
				state=st_normal;
				break;
		}
	}

	return r;
}

QString cgi_escape (const QString &text, bool leave_high)
	// leave_high: leave alone characters>127. This is incompatible with CGI,
	// but useful for using the escaping mechanism in other places.
{
	QString r;

	QString::const_iterator e=text.end ();
	for (QString::const_iterator it=text.begin (); it!=e; ++it)
	{
		unsigned char ch=(*it).toAscii();
		if (ch==' ')
			r+="+";
		else if (
			ch=='='||
			ch=='+'||
			ch=='&'||
			ch=='%'||
			ch=='#'||
			(!leave_high && ch>127)||
			ch<32)
			r+="%"+hex_string (ch);
		else
			r+=QString (1, ch);
	}

	return r;
}

QString bool_to_string (bool val, const QString &true_value, const QString &false_value)
{
	if (val)
		return true_value;
	else
		return false_value;
}

bool string_to_bool (const QString &text)
{
	if (text.toInt ()==0)
		return false;
	else
		return true;
}

QString make_string (const QSet<QString> s, const QString &separator)
{
	QString r;

	QSet<QString>::const_iterator end=s.end ();
	for (QSet<QString>::const_iterator it=s.begin (); it!=end; ++it)
	{
		if (it!=s.begin ()) r.append (separator);
		r.append (*it);
	}

	return r;
}


std::ostream &operator<< (std::ostream &s, const QString &c)
{
	return s << q2std (c);
}



