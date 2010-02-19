#include "text.h"

#include <QStringList>

#include <stdlib.h>
#include <iostream>

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
	char *r=getenv (name.latin1());
	if (r)
		return QString (r);
	else
		return QString ();
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
	return s << qPrintable (c);
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




bool yesNoQuestion (QWidget *parent, QString title, QString question)
{
	// TODO: Yes/No, but with ESC
	QMessageBox::StandardButtons buttons=QMessageBox::Ok | QMessageBox::Cancel;
	QMessageBox::StandardButton result=QMessageBox::question (parent, title, question, buttons, QMessageBox::Ok);
	return result==QMessageBox::Ok;
}

bool confirmProblem (QWidget *parent, const QString title, const QString problem)
	/*
	 * Displays a message and ask the user if he wishes to accept anyway.
	 * Parameters:
	 *   - parent: passed on to the QMessageBox constructor.
	 *   - msg: the message.
	 * Return value:
	 *   - if the user accepted.
	 */
{
	// TODO: Buttons Yes/No, but with Esc handling
	QString question=problem+" Trotzdem akzeptieren?";
	return yesNoQuestion (parent, title, question);
}

bool confirmProblem (QWidget *parent, const QString problem)
{
	return confirmProblem (parent, "Warnung", problem);
}

QString firstToUpper (const QString &text)
{
	return text.left (1).toUpper ()+text.mid (1);
}

QString firstToLower (const QString &text)
{
	return text.left (1).toLower ()+text.mid (1);
}

/** Converts a std::string to a QString */
QString std2q (std::string s)
{
	return QString (s.c_str ());
}

/** Converts a QString to a std::string */
std::string q2std (QString s)
{
	if (s.isNull ()) return "";
	return std::string (s.toLatin1 ().constData ());
}
