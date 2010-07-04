/*
 * InfoPluginFactory.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPluginFactory.h"

#include <QList>
#include <QDebug>

InfoPluginFactory *InfoPluginFactory::instance;

InfoPluginFactory::InfoPluginFactory ()
{
}

InfoPluginFactory::~InfoPluginFactory ()
{
	foreach (InfoPlugin::Descriptor *descriptor, descriptors)
		delete descriptor;
}

InfoPluginFactory &InfoPluginFactory::getInstance ()
{
	if (!InfoPluginFactory::instance)
		InfoPluginFactory::instance=new InfoPluginFactory;

	return *InfoPluginFactory::instance;
}

void InfoPluginFactory::addDescriptor (InfoPlugin::Descriptor *descriptor)
{
	qDebug () << QString ("Registering plugin %1").arg (descriptor->getName ());

	descriptors.append (descriptor);
}

InfoPluginFactory::Registration::Registration (InfoPlugin::Descriptor *descriptor)
{
	InfoPluginFactory::getInstance ().addDescriptor (descriptor);
}
