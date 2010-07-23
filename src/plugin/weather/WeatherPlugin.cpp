/*
 * WeatherPlugin.cpp
 *
 *  Created on: 23.07.2010
 *      Author: Martin Herrmann
 */

#include "WeatherPlugin.h"

WeatherPlugin::WeatherPlugin ()
{
}

WeatherPlugin::~WeatherPlugin ()
{
}

void WeatherPlugin::outputText (const QString &text, Qt::TextFormat format)
{
	emit textOutput (text, format);
}

void WeatherPlugin::outputImage (const QImage &image)
{
	emit imageOutput (image);
}


// ****************
// ** Descriptor **
// ****************

bool WeatherPlugin::Descriptor::nameLessThan (const WeatherPlugin::Descriptor &d1, const WeatherPlugin::Descriptor &d2)
{
	return d1.getName () < d2.getName ();
}

bool WeatherPlugin::Descriptor::nameLessThanP (const WeatherPlugin::Descriptor *d1, const WeatherPlugin::Descriptor *d2)
{
	return nameLessThan (*d1, *d2);
}
