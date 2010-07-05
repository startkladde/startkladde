/*
 * InfoPlugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPlugin.h"

#include <QSettings>
#include <QDebug>

InfoPlugin::InfoPlugin ():
	enabled (true)
{
}

InfoPlugin::~InfoPlugin ()
{
}

void InfoPlugin::outputText (const QString &text)
{
	emit textOutput (text);
}

void InfoPlugin::loadSettings (const QSettings &settings)
{
	caption=settings.value ("caption", getName ()).toString ();
	enabled=settings.value ("enabled", true).toBool ();
}

void InfoPlugin::saveSettings (QSettings &settings)
{
	settings.setValue ("caption", caption);
	settings.setValue ("enabled", enabled);
}
