/*
 * InfoPlugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPlugin.h"

#include <QSettings>

InfoPlugin::InfoPlugin ()
{
}

InfoPlugin::~InfoPlugin ()
{
}

const QString &InfoPlugin::getText () const
{
	return text;
}

void InfoPlugin::outputText (const QString &text)
{
	this->text=text;
	emit textOutput (text);
}

const QString &InfoPlugin::getCaption () const
{
	return caption;
}

void InfoPlugin::setCaption (const QString &caption)
{
	this->caption=caption;
}


void InfoPlugin::loadSettings (const QSettings &settings)
{
	caption=settings.value ("caption").toString ();
}

void InfoPlugin::saveSettings (QSettings &settings)
{
	settings.setValue ("caption", caption);
}
