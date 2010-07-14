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
	{
		QString line=QString::fromUtf8 (file.readLine ().constData ());
		if (line.trimmed ().contains (regexp))
    		return true;
	}

	return false;
}

/**
 * The passed QRegExp is copied. Use the other findInFile function to access
 * the RegExp.
 *
 * Throws FileOpenError
 *
 * @param filename
 * @param regexp
 * @return
 */
QString findInFile (const QString &filename, const QRegExp &regexp, int group)
{
	// Make a copy because apparenly we cannot capture in a const QRegExp (but
	// we want to pass a const& so we can use an anonymous value in calls).
	QRegExp re (regexp);

	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
		throw FileOpenError (filename, file.error (), file.errorString ());

	while (!file.atEnd ())
	{
		QString line=QString::fromUtf8 (file.readLine ().constData ());
    	if (line.trimmed ().contains (re))
    		return re.cap (group);
	}

	return QString ();
}
