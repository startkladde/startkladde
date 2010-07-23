/*
 * Plugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "Plugin.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/environment.h"

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

/**
 * Determines whether a file name is absolute
 *
 * A file name is considered absolute if QFileInfo::isAbsolute returns true, or
 * if the file name starts with "./" (or ".\\").
 *
 * @param filename a file name
 * @return true if filename is absolute, false if not
 */
bool Plugin::filenameIsAbsolute (const QString &filename) const
{
	// We treat file names with the explicit directory ./ as absolute
	if (filename.startsWith ("./")) return true;
	if (filename.startsWith (".\\")) return true;
	if (QFileInfo (filename).isAbsolute ()) return true;

	return false;
}

/**
 *
 * @param filename
 * @param pluginPaths usually, use Settings::instance ().pluginPaths
 * @return
 */
QString Plugin::resolveFilename (const QString &filename, const QStringList &pluginPaths) const
{
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

	// Search in the system path
	QStringList systemPath=getSystemPath ();
	foreach (const QString &path, systemPath)
	{
		QString full=path+"/"+filename;
		if (QFile::exists (full))
			return full;
	}


	// Not found
	return QString ();
}
