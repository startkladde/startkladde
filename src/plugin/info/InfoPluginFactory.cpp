/*
 * InfoPluginFactory.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPluginFactory.h"

#include <QList>


// ******************
// ** Construction **
// ******************

InfoPluginFactory::InfoPluginFactory ()
{
}

InfoPluginFactory::~InfoPluginFactory ()
{
	foreach (InfoPlugin::Descriptor *descriptor, descriptors)
		delete descriptor;
}


// ***************
// ** Singleton **
// ***************

InfoPluginFactory *InfoPluginFactory::instance;

InfoPluginFactory &InfoPluginFactory::getInstance ()
{
	if (!InfoPluginFactory::instance)
		InfoPluginFactory::instance=new InfoPluginFactory;

	return *InfoPluginFactory::instance;
}

// *************
// ** Factory **
// *************

void InfoPluginFactory::addDescriptor (InfoPlugin::Descriptor *descriptor)
{
	descriptors.append (descriptor);
}

const QList<InfoPlugin::Descriptor *> &InfoPluginFactory::getDescriptors ()
{
	return descriptors;
}

const InfoPlugin::Descriptor *InfoPluginFactory::find (const QString &id) const
{
	// FIXME use a hash/map
	foreach (InfoPlugin::Descriptor *descriptor, descriptors)
		if (descriptor->getId ()==id)
			return descriptor;

	return NULL;
}

InfoPlugin *InfoPluginFactory::create (const QString &id, const QString &caption) const
{
	const InfoPlugin::Descriptor *descriptor=find (id);
	if (!descriptor) return NULL;

	InfoPlugin *plugin=descriptor->create ();
	if (!plugin) return NULL;

	plugin->setCaption (caption);

	return plugin;
}


// *************************************
// ** InfoPluginFactory::Registration **
// *************************************

InfoPluginFactory::Registration::Registration (InfoPlugin::Descriptor *descriptor)
{
	InfoPluginFactory::getInstance ().addDescriptor (descriptor);
}
