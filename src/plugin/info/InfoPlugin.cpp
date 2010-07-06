/*
 * InfoPlugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPlugin.h"

#include <QSettings>
#include <QDebug>

#include "src/plugin/info/InfoPluginSettingsPane.h"

InfoPlugin::InfoPlugin ():
	enabled (true)
{
}

InfoPlugin::~InfoPlugin ()
{
}

void InfoPlugin::outputText (const QString &text, Qt::TextFormat format)
{
	emit textOutput (text, format);
}

/**
 * Returns an InfoPluginSettingsPane with fields for the caption and the pane
 * created by infoPluginCreateSettingsPane
 *
 * The caller takes ownership of the created PluginSettingsPane.
 *
 * @param parent the parent widget
 * @return a newly created PluginSettingsPane
 */
PluginSettingsPane *InfoPlugin::createSettingsPane (QWidget *parent)
{
	return new InfoPluginSettingsPane (this, parent);
}

/**
 * Reads the common settings for this info plugin
 *
 * Implementations of this class will generally want to override this method.
 * The overridden method must call this method so the common settings will be
 * read.
 *
 * @param settings the QSettings to read the settings from
 * @see Plugin::readSettings
 */
void InfoPlugin::readSettings (const QSettings &settings)
{
	// FIXME use a infoPluginReadSettings instead?
	caption=settings.value ("caption", getName ()).toString ();
	enabled=settings.value ("enabled", true).toBool ();
}

/**
 * Writes the common settings for this info plugin
 *
 * Implementations of this class will generally want to override this method.
 * The overridden method must call this method so the common settings will be
 * written.
 *
 * @param settings the QSettings to write the settings to
 * @see Plugin::writeSettings
 */
void InfoPlugin::writeSettings (QSettings &settings)
{
	settings.setValue ("caption", caption);
	settings.setValue ("enabled", enabled);
}
