/*
 * qDate.cpp
 *
 *  Created on: 09.08.2010
 *      Author: martin
 */

#include "qDate.h"

#include <QDate>

/**
 * Formats a date with a default format specification
 *
 * @param date the date to format
 * @param zeroPad whether to pad the components to a constant width with zeros
 * @return a string containing the formatted date, or an invalid string if the
 *         date is invalid
 */
QString toString (const QDate &date, bool zeroPad)
{
	if (date.isValid ())
		return date.toString (zeroPad?"dd.MM.yyyy":"d.M.yyyy");
	else
		return QString ();
}
