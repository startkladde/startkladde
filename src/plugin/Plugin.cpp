/*
 * Plugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "Plugin.h"

#include <QFile>

#include "src/text.h"

Plugin::Plugin ()
{
}

Plugin::~Plugin ()
{
}

void Plugin::restart ()
{
	terminate ();
	start ();
}

bool Plugin::filenameIsAbsolute (const QString &filename) const
{
	// FIXME Windows
	return
		filename.trimmed ().startsWith ("/") ||
		filename.trimmed ().startsWith ("./");
}

// FIXME no default
QString Plugin::resolveFilename (const QString &filename, const QStringList &pluginPaths) const
{
	// FIXME Windows

	if (isBlank (filename))
		return "";

	// Absolute file names are not changed
	if (filenameIsAbsolute (filename))
		return filename;

	// Search in the plugin paths
	foreach (const QString &path, pluginPaths)
	{
		QString full=path+"/"+filename;
		if (QFile::exists (full))
			return full;
	}

	// Search in the current directory
	if (QFile::exists (filename))
		return filename;

	// Not found
	return QString ();
}
