/*
 * InfoPluginFactory.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPluginFactory.h"

#include <QList>

#include "src/util/qList.h"

// ******************
// ** Construction **
// ******************

InfoPluginFactory::InfoPluginFactory ()
{
}

InfoPluginFactory::~InfoPluginFactory ()
{
	deleteList (descriptors);
}


// ***************
// ** Singleton **
// ***************

InfoPluginFactory *InfoPluginFactory::instance;

/**
 * Returns the singleton instance of InfoPluginFactory
 *
 * @return the singleton instance of InfoPluginFactory
 */
InfoPluginFactory &InfoPluginFactory::getInstance ()
{
	if (!InfoPluginFactory::instance)
		InfoPluginFactory::instance=new InfoPluginFactory;

	return *InfoPluginFactory::instance;
}

// *************
// ** Factory **
// *************

/**
 * Registers an info plugin by adding its descriptor to the list of know info
 * plugin descriptors
 *
 * @param descriptor the descriptor for the info plugin to add
 */
void InfoPluginFactory::addDescriptor (InfoPlugin::Descriptor *descriptor)
{
	descriptors.append (descriptor);
}

/**
 * Returns the list of all known info plugin descriptors
 *
 * @return a list of pointers to info plugin descriptors. The caller may not
 *         delete any of the pointers.
 */
const QList<InfoPlugin::Descriptor *> &InfoPluginFactory::getDescriptors ()
{
	return descriptors;
}

/**
 * Finds the descriptor for a plugin with the given ID
 *
 * If no plugin with the given ID has been registered, NULL is returned. If
 * multiple plugins with the same ID have been registered, an arbitrary one of
 * them is returned.
 * FIXME prevent this
 *
 * @param id the ID of the plugin to find
 * @return a pointer to the descriptor for the plugin, or NULL
 */
const InfoPlugin::Descriptor *InfoPluginFactory::find (const QString &id) const
{
	// FIXME use a hash/map
	foreach (InfoPlugin::Descriptor *descriptor, descriptors)
		if (descriptor->getId ()==id)
			return descriptor;

	return NULL;
}

/**
 * Creates an info plugin with a given ID and caption
 *
 * This is a shortcut for retrieving the descriptor, calling its create method
 * and setting the caption. Usually, the caption will be read from the settings
 * along with other settings of the plugin, so this method is only useful for
 * creating a sample plugin instance.
 *
 * If no plugin with the given ID has been registered, NULL is returned.
 *
 * The caller takes ownership of the returned InfoPlugin instance.
 *
 * @param id the ID of the plugin to create
 * @param caption the caption of the plugin to set
 * @return an InfoPlugin instance, or NULL
 * @see #find
 */
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

/**
 * Registers an info plugin
 *
 * @param descriptor the descriptor of the plugin to register
 */
InfoPluginFactory::Registration::Registration (InfoPlugin::Descriptor *descriptor)
{
	InfoPluginFactory::getInstance ().addDescriptor (descriptor);
}
