/*
 * InfoPlugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPlugin.h"

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

const QString &InfoPlugin::getTitle () const
{
	return title;
}

void InfoPlugin::setTitle (const QString &title)
{
	this->title=title;
}
