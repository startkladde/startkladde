/*
 * file.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "file.h"

#include <QRegExp>

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
