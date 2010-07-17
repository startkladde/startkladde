/*
 * io.cpp
 *
 *  Created on: 17.07.2010
 *      Author: Martin Herrmann
 */

#include "io.h"

#include <QRegExp>
#include <QIODevice>
#include <QString>

bool findInIoDevice (QIODevice &device, QRegExp &regexp)
{
	while (!device.atEnd ())
	{
		QString line=QString::fromUtf8 (device.readLine ().constData ());
		if (line.trimmed ().contains (regexp))
    		return true;
	}

	return false;
}

QString findInIoDevice (QIODevice &device, const QRegExp &regexp, int group)
{
	// Make a copy because apparenly we cannot capture in a const QRegExp (but
	// we want to pass a const& so we can use an anonymous value in calls).
	QRegExp re (regexp);

	if (findInIoDevice (device, re))
		return re.cap (group);
	else
		return QString ();
}
