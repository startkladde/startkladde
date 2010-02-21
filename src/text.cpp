#include "text.h"

#include <stdlib.h>
#include <iostream>

#include <QStringList>

#include "config/Options.h"

const QString whitespace=" \t\r\n";

using std::cout; using std::cerr; using std::endl;

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

bool eintraege_sind_leer (QString eintrag1, QString eintrag2)
{
	return eintrag_ist_leer (eintrag1) && eintrag_ist_leer (eintrag2);
}

// TODO move somewhere more appropriate, then remove dependency on options.h
bool airfieldEntryCanBeChanged (QString airfield)
{
	return eintrag_ist_leer (airfield) || airfield.simplified () == opts.ort.simplified ();
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

QString get_environment (const QString& name)
{
	char *r=getenv (name.utf8 ().constData ());
	if (r)
		return QString (r);
	else
		return QString ();
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


std::ostream &operator<< (std::ostream &s, const QColor &c)
{
	return s << c.red () << "," <<  c.green () << "," << c.blue ();
}

std::ostream &operator<< (std::ostream &s, const QByteArray &ba)
{
	int l=ba.length ();
	for (int i=0; i<l; ++i)
		s << ba.at (i);

	return s;
}




QString firstToUpper (const QString &text)
{
	return text.left (1).toUpper ()+text.mid (1);
}

QString firstToLower (const QString &text)
{
	return text.left (1).toLower ()+text.mid (1);
}
