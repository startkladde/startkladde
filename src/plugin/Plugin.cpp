/*
 * Plugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "Plugin.h"

#include <QFile>

#include "src/config/Settings.h"
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

QString Plugin::resolveFilename (const QString &filename) const
{
	// FIXME Windows

	if (isBlank (filename))
		return "";

	// Absolute file names are not changed
	if (filenameIsAbsolute (filename))
		return filename;

	// Search in the plugin paths
	// FIXME will not reflect current changes
	foreach (const QString &path, Settings::instance ().pluginPaths)
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
