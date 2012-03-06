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

QString dateRangeToString (const QDate &first, const QDate &last, const QString &separator, Qt::DateFormat format)
{
	if (!first.isValid () || !last.isValid ())
		return qApp->translate ("QDate", "invalid");
	else if (first==last)
		return first.toString (format);
	else
		return first.toString (format)+separator+last.toString (format);
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
	return s << date.toString (Qt::ISODate);
}
