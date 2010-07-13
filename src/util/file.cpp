/*
 * file.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "file.h"

#include <QRegExp>
#include <QDebug>

/**
 * Throws FileOpenError
 *
 * @param filename
 * @param regexp
 * @return
 */
bool findInFile (const QString &filename, QRegExp &regexp)
{
	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
		throw FileOpenError (filename, file.error (), file.errorString ());

	while (!file.atEnd ())
    	if (QString (file.readLine ()).trimmed ().contains (regexp))
    		return true;

	return false;
}

/**
 * Throws FileOpenError
 *
 * @param filename
 * @param regexp
 * @return
 */
QString findInFile (const QString &filename, const QRegExp &regexp, int group)
{
	// Make a copy because we cannot capture in a const QRegExp (but we want
	// to pass a const& so we can use an anonymous value in calls)

	QRegExp re (regexp);

	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
		throw FileOpenError (filename, file.error (), file.errorString ());

	while (!file.atEnd ())
    	if (QString (file.readLine ()).trimmed ().contains (re))
    		return re.cap (group);

	return QString ();
}
