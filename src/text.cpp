#include "text.h"

#include <QString>
#include <QStringList>
#include <QAbstractButton>

#include "src/config/Settings.h"
#include "src/notr.h"

const QString whitespace=notr (" \t\r\n");

/**
 * Determines whether a string consists only of whitespace
 */
bool isBlank (const QString &string)
{
	return string.trimmed ().isEmpty ();
}

/**
 * Determines whether string designates "none"
 *
 * An entry is considered "none" if it is blank or "-".
 *
 * @param string a string
 * @return true if the string designates "none"
 */
bool isNone (const QString &string)
{
	QString trimmed=string.trimmed ();
	return (trimmed.isEmpty () || string==notr ("-"));
}

/**
 * Determines whether two strings designate "none", as determined by isNone
 * (const QString &)
 *
 * @param eintrag1 a string
 * @param eintrag2 another string
 * @return true if both strings designates "none"
 */
bool isNone (const QString &string1, const QString &string2)
{
	return isNone (string1) && isNone (string2);
}

/**
 * Trims all strings of a list
 *
 * More precisely, replaces all strings of a list with the result of their
 * trimmed method.
 */
void trim (QStringList &strings)
{
	QMutableStringListIterator it (strings);
	while (it.hasNext ())
	{
		it.next ();
		it.value()=it.value().trimmed();
	}
}

/**
 * Converts the first character of a string to upper case
 */
QString firstToUpper (const QString &string)
{
	return string.left (1).toUpper ()+string.mid (1);
}

/**
 * Converts the first character of a string to lower case
 */
QString firstToLower (const QString &text)
{
	return text.left (1).toLower ()+text.mid (1);
}

/**
 * Converts the first character of a string to upper case and the rest of the
 * string to lower case
 */
QString capitalize (const QString &string)
{
	return string.left (1).toUpper() + string.mid (1).toLower ();
}

/**
 * Generates a string in singular or plural form. Note that for translated
 * strings, tr() in the plural form should be used instead because it handles
 * languages with different distinctions than singular and plural.
 *
 * @param count the number of objects
 * @param singular the singular form of the string
 * @param plural the plural form of the string with %1 for the count
 * @return if count is 1, the singular string; otherwise, the plural string
 *         with %1 filled in with the count
 */
QString countText (int count, const QString &singular, const QString &plural)
{
	if (count==1)
		return singular;
	else
		return plural.arg (count);
}

QString countText (int count, const QString &singular, const QString &plural, const QString &none)
{
	if (count==0)
		return none;
	else if (count==1)
		return singular;
	else
		return plural.arg (count);
}

/**
 * Creates a canonical form of a club name useful for comparison
 *
 * The club name is simplified and converted to lower case.
 */
QString simplifyClubName (const QString &clubName)
{
	return clubName.simplified ().toLower ();
}

/**
 * Determines whether it is OK to overwrite a location entry in an input field
 *
 * @param location the location entry
 * @return true if it may be overwritten
 */
// TODO move somewhere more appropriate, then remove dependency on Settings.h
bool locationEntryCanBeChanged (const QString &location)
{
	return
		isNone (location) ||
		location.simplified ().toLower () == Settings::instance ().location.simplified ().toLower ();
}

QString insertMnemonic (const QString &text, const QString &disallowed, bool fallbackToFirst)
{
	QString disallowedLower=disallowed.toLower ();
	QString textLower=text.toLower ();

	QString result=text;

	// Improvement: use std::string functionality (wrap for QString)
	for (int i=0; i<text.length (); ++i)
		if (!disallowedLower.contains (textLower[i]))
			return result.insert (i, notr ("&"));

	// No allowed mnemonic possible
	if (fallbackToFirst)
		return qnotr ("&")+text;
	else
		return text;
}

QChar getMnemonic (const QString &text)
{
	int ampersandPosition=text.indexOf ('&');

	if (ampersandPosition<0)
		// No ampersand in string
		return QChar ();
	else if (ampersandPosition+1>=text.length ())
		// Ampersand is last character (or beyond the end of the string)
		return QChar ();
	else
		// Ampersand found
		return text.at (ampersandPosition+1);
}

QChar getMnemonic (const QAbstractButton *button)
{
	if (button)
		return getMnemonic (button->text ());
	else
		return QChar ();
}

QString repeatString (const QString &string, unsigned int num, const QString &separator)
{
	if (num==0) return QString ();

	QString result=string;

	for (unsigned int i=0; i<num-1; ++i)
		result+=separator+string;

	return result;
}
