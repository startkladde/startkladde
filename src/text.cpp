#include "text.h"
#include <list>
#include <stdlib.h>

const string whitespace=" \t\r\n";

string trim (const string& s)/*{{{*/
	/*
	 * Cuts off leading and trailing whitespace.
	 * Parameters:
	 *   - s: the original string. Unmodified.
	 * Return value:
	 *   - the string without leading or trailing whitespace.
	 */
{
	if (s.length ()==0) return s;
	int first=s.find_first_not_of (whitespace);
	int last=s.find_last_not_of (whitespace);
	if (first<0) return "";
	int len=last-first+1;
	return string (s, first, len);
}
/*}}}*/

bool eintrag_ist_leer (const QString &eintrag)/*{{{*/
	/*
	 * Finds out whether an entry is considered "empty".
	 * Parameters:
	 *   - eintrag: the string.
	 * Return value:
	 *   if eintrag is to be considered "empty".
	 */
{
	// QT3:
	//QString e=eintrag.stripWhiteSpace ();
	// QT4:
	QString e=eintrag.simplified ();
	return (e=="" || e=="-");
}
/*}}}*/

bool eintrag_ist_leer (const string &eintrag)/*{{{*/
	/*
	 * Wrapper for eintrag_ist_leer (QString), taking a std::string instead.
	 * Parameters:
	 *   - eintrag: converted an passed on.
	 * Return value:
	 *   - passed on.
	 */
{
	return eintrag_ist_leer (std2q (eintrag));
}
/*}}}*/

bool check_message (QWidget *parent, const string &msg)/*{{{*/
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
			std2q (msg)+"Trotzdem akzeptieren?", "&Ja", "&Nein", QString::null, 0, 1)
			!=QDialog::Accepted;
	return (ret==QDialog::Accepted);
}
/*}}}*/

string t_pilot_bezeichnung (flug_typ flugtyp, casus c)/*{{{*/
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
		case ft_schul_2: return string ("Flugschüler"); break;
		default:
			switch (c)
			{
				case cas_nominativ: return "Pilot"; break;
				case cas_akkusativ: return "Piloten"; break;
				default: return "[Pilot]"; break;
			}
			break;
	}
}
/*}}}*/

string t_begleiter_bezeichnung (flug_typ flugtyp, casus c)/*{{{*/
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
		case ft_schul_2: return "Fluglehrer"; break;
		case ft_gast_privat: case ft_gast_extern: return "Gast"; break;
		default: return "Begleiter"; break;
	}
}
/*}}}*/

void replace_substring (string &s, const string& substr, const string& replacement)/*{{{*/
	/*
	 * Replaces every occurance of a substring in a string with some other string.
	 * Slow. Undefined for overlaps.
	 * Parameters:
	 *   - s: the string to search in.
	 *   - substr: the string to search for.
	 *   - replacement: the string substr is replaced with.
	 */
{
	for (string::size_type pos=0;
		(pos=s.find (substr))!=string::npos;
		pos++)
		s.replace (pos, substr.length (), replacement);
}
/*}}}*/

void replace_tabs (string &s)/*{{{*/
{
	replace_substring (s, "\t", " ");
}
/*}}}*/

string num_to_string (int num, unsigned int min_length, char pad)/*{{{*/
{
	ostringstream oss;
	oss << num;
	string r=oss.str ();
	if (r.length ()<min_length) r=string (min_length-r.length (), pad)+r;
	return r;
}
/*}}}*/

string num_to_string (float num)/*{{{*/
{
	ostringstream oss;
	oss << num;
	string r=oss.str ();
	return r;
}
/*}}}*/

string num_to_string (unsigned int num)/*{{{*/
{
	ostringstream oss;
	oss << num;
	return oss.str ();
}
/*}}}*/

string num_to_string (db_id num)/*{{{*/
{
	ostringstream oss;
	oss << num;
	return oss.str ();
}
/*}}}*/

//string text::num_to_string (size_t num)/*{{{*/
//{
//	ostringstream oss;
//	oss << num;
//	return oss.str ();
//}
/*}}}*/

QString simplify_club_name (const QString& s)/*{{{*/
{
	// QT3:
	//QString r=s.simplifyWhiteSpace ().lower ();
	// QT4:
	QString r=s.simplified ().toLower ();
	if (eintrag_ist_leer (r))
		return "-";
	else
		return r;
}
/*}}}*/

string simplify_club_name (const string& s)/*{{{*/
{
	return q2std (simplify_club_name (std2q (s)));
}
/*}}}*/

bool club_name_identical (const string& s1, const string& s2)/*{{{*/
{
	return simplify_club_name (s1)==simplify_club_name (s2);
}
/*}}}*/

string concatenate_comments (const string& s1, const string& s2)/*{{{*/
{
	if (eintrag_ist_leer (s1)) return s2;
	if (eintrag_ist_leer (s2)) return s1;
	return s1+"; "+s2;
}
/*}}}*/

string string_or_none (const string& text)/*{{{*/
{
	if (eintrag_ist_leer (text))
		return "-";
	else
		return text;
}
/*}}}*/

bool parameter_matches (char *parameter, char *val1, char *val2)/*{{{*/
{
	return ((strcmp (parameter, val1)==0) || (strcmp (parameter, val2)==0));
}/*}}}*/

#include <iostream>

void split_string (list<string> &strings, const string &separators, const string &text)/*{{{*/
{
	string::size_type start_pos=0;
	string::size_type end_pos;

	if (text.empty ()) return;

	while (end_pos=text.find_first_of (separators, start_pos), end_pos!=string::npos)
	{
		strings.push_back (text.substr (start_pos, end_pos-start_pos));
		start_pos=end_pos+1;
	}
	strings.push_back (text.substr (start_pos));
}
/*}}}*/

void split_string (string &string1, string &string2, const string &separators, const string &text)/*{{{*/
	// Splits a string at the first occurence of one of separators. Results in string1 and string2
{
	string::size_type pos=text.find_first_of (separators);
	
	if (pos==string::npos)
	{
		// No separator found
		string1=text;
		string2="";
	}
	else
	{
		// abc=defgh
		// 012345678
		// pos=3
		// l=9
		// l1=3
		// l2=6
		//
		string1=text.substr (0, pos);
		if (pos>=text.length ()-1)
			string2="";
		else
			string2=text.substr (pos+1);
	}
}
/*}}}*/

void trim (list<string> &strings)/*{{{*/
{
	list<string>::iterator end=strings.end ();
	for (list<string>::iterator it=strings.begin (); it!=end; ++it)
		(*it)=trim (*it);
}
/*}}}*/

bool is_date (const string& s)/*{{{*/
	/*
	 * Finds out if a string can be parsed as date.
	 * Parameters:
	 *   - s: the string to test.
	 * Return value:
	 *   - true if s is a date.
	 *   - false else.
	 */
{
	// TODO remove as the result is discarded.
	return QDate::fromString (std2q (s), Qt::ISODate).isValid ();
}
/*}}}*/

string latex_escape (const string& o)/*{{{*/
	/*
	 * Escape a string for putting to a LaTeX document.
	 * Parameters:
	 *   - o: the string to escape.
	 * Return value:
	 *   - the escaped string.
	 */
{
	ostringstream oss;

	for (unsigned int i=0; i<o.length (); i++)
	{
		switch (o[i])
		{
			case '$': oss << "\\$"; break;
			case '\\': oss << "\\textbackslash{}"; break;
			case '%': oss << "\\%"; break;
			case '_': oss << "\\_"; break;
			case '{': oss << "\\{"; break;
			case '&': oss << "\\&"; break;
			case '#': oss << "\\#"; break;
			case '}': oss << "\\}"; break;
			case '^': oss << "\\textasciicircum{}"; break;
			case '~': oss << "\\textasciitilde{}"; break;
			case '"': oss << "\\textquotedbl{}"; break;
			// Seems like guilsingl{left,right} don't work properly
			//case '<': oss << "\\guilsinglleft{}"; break;
			//case '>': oss << "\\guilsinglright{}"; break;
			default: oss << o[i]; break;
		}
	}

	return oss.str ();
}/*}}}*/

/**
  * Escape a string for putting to a LaTeX document.
  * Parameters:
  *   - o: the string to escape.
  * Return value:
  *   - the escaped string.
  */
string csv_escape (const string& o)
{
	// TODO werden ""en korrekt behandelt?
	ostringstream oss;

	char quote_char='"';

	oss << quote_char;
	for (unsigned int i=0; i<o.length (); i++)
	{
		if (o[i]==quote_char)
			oss << quote_char << quote_char;
		else
			oss << o[i];
	}
	oss <<quote_char;

	return oss.str ();
}

string html_escape (const string& s, bool also_escape_newlines)/*{{{*/
{
	string r;

	string::const_iterator end=s.end ();
	for (string::const_iterator ch=s.begin (); ch!=end; ++ch)
	{
		if (*ch=='<') r+="&lt;";
		else if (*ch=='>') r+="&gt;";
		else if (*ch=='"') r+="&quot;";
		// Don't escape spaces because this will break (hidden) CGI arguments.
//		else if (*ch==' ') r+="&nbsp;";
		else if (*ch=='&') r+="&amp;";
		else if (also_escape_newlines && *ch=='\n') r+="<br>";
		else r+=string (1, *ch);
	}

	return r;
}
/*}}}*/

list<string> html_escape (const list<string> &l, bool also_escape_newlines)/*{{{*/
{
	list<string> r;
	list<string>::const_iterator end=l.end ();
	for (list<string>::const_iterator it=l.begin (); it!=end; ++it)
		r.push_back (html_escape (*it, also_escape_newlines));

	return r;
}
/*}}}*/

string get_environment (const string& name)
{
	char *r=getenv (name.c_str ());
	if (r)
		return string (r);
	else
		return string ();
}


char hex_digit_value (char digit)/*{{{*/
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
/*}}}*/

string hex_digit (char value)/*{{{*/
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
/*}}}*/

string hex_string (unsigned char value)/*{{{*/
{
	return hex_digit (value/16)+hex_digit (value%16);
}
/*}}}*/

string cgi_unescape (const string &text)/*{{{*/
{
	enum state_t { st_normal, st_expect_first, st_expect_second };
	state_t state=st_normal;

	string r;
	char special;

	string::const_iterator e=text.end ();
	for (string::const_iterator it=text.begin (); it!=e; ++it)
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
					r+=string (1, *it);
				break;
			case st_expect_first:
				special=16*hex_digit_value (*it);
				state=st_expect_second;
				break;
			case st_expect_second:
				special+=hex_digit_value (*it);
				r+=string (1, special);
				state=st_normal;
				break;
		}
	}

	return r;
}
/*}}}*/

string cgi_escape (const string &text, bool leave_high)/*{{{*/
	// leave_high: leave alone characters>127. This is incompatible with CGI,
	// but useful for using the escaping mechanism in other places.
{
	string r;

	string::const_iterator e=text.end ();
	for (string::const_iterator it=text.begin (); it!=e; ++it)
	{
		unsigned char ch=(*it);
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
			r+=string (1, ch);
	}

	return r;
}
/*}}}*/

string bool_to_string (bool val, const string &true_value, const string &false_value)/*{{{*/
{
	if (val)
		return true_value;
	else
		return false_value;
}
/*}}}*/

bool string_to_bool (const string &text)/*{{{*/
{
	if (atoi (text.c_str ())==0)
		return false;
	else
		return true;
}
/*}}}*/

bool starts_with (const string &text, const string &substring)/*{{{*/
{
	if (text.substr (0, substring.length ())==substring)
		return true;
	else
		return false;
}
/*}}}*/

bool ends_with (const string &text, const string &substring)/*{{{*/
{
	if (text.substr (text.length ()-substring.length ())==substring)
		return true;
	else
		return false;
}
/*}}}*/

string make_string (const set<string> s, const string &separator)/*{{{*/
{
	string r;

	set<string>::const_iterator end=s.end ();
	for (set<string>::const_iterator it=s.begin (); it!=end; ++it)
	{
		if (it!=s.begin ()) r.append (separator);
		r.append (*it);
	}

	return r;
}
/*}}}*/


