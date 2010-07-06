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

PluginSettingsPane *InfoPlugin::createSettingsPane (QWidget *parent)
{
	return new InfoPluginSettingsPane (this, parent);
}

void InfoPlugin::readSettings (const QSettings &settings)
{
	caption=settings.value ("caption", getName ()).toString ();
	enabled=settings.value ("enabled", true).toBool ();
}

void InfoPlugin::writeSettings (QSettings &settings)
{
	settings.setValue ("caption", caption);
	settings.setValue ("enabled", enabled);
}
