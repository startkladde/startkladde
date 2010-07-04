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

const QString &InfoPlugin::getText ()
{
	return text;
}

void InfoPlugin::outputText (const QString &text)
{
	this->text=text;
	emit textOutput (text);
}
