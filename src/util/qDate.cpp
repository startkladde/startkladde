/*
 * qDate.cpp
 *
 *  Created on: 09.08.2010
 *      Author: martin
 */

#include "qDate.h"

#include <QApplication>
#include <QDate>

#include "src/util/qString.h"

/**
 * Returns date if it is valid, or invalidOption if date is not valid
 */
QDate validDate (const QDate &date, const QDate &invalidOption)
{
	if (date.isValid ())
		return date;
	else
		return invalidOption;
}

/**
 * Formats a date with a default format specification
 *
 * @param date the date to format
 * @param zeroPad whether to pad the components to a constant width with zeros
 * @return a string containing the formatted date, or an invalid string if the
 *         date is invalid
 */
// TODO TR: better localized format
QString toString (const QDate &date, bool zeroPad)
{
	if (date.isValid ())
	{
		if (zeroPad)
			return date.toString (qApp->translate ("QDate", "MM/dd/yyyy"));
		else
			return date.toString (qApp->translate ("QDate", "M/d/yyyy"));
	}
	else
	{
		return QString ();
	}
}

QString toString (const QDate &first, const QDate &last, const QString &separator, bool zeroPad)
{
	if (!first.isValid () || !last.isValid ())
		return (qApp->translate ("QDate", "invalid"));
	else if (first==last)
		return toString (first, zeroPad);
	else
		return toString (first, zeroPad)+separator+toString (last, zeroPad);
}

QDate firstOfYear (int year)
{
	return QDate (year, 1, 1);
}

QDate firstOfYear (const QDate &date)
{
	return firstOfYear (date.year ());
}

std::ostream &operator<< (std::ostream &s, const QDate &date)
{
	return s << toString (date, false);
}
