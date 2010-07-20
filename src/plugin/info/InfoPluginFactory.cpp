/*
 * InfoPluginFactory.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPluginFactory.h"

#include <iostream>

#include <QList>

#include "src/util/qList.h"
#include "src/util/qString.h"

// ******************
// ** Construction **
// ******************

InfoPluginFactory::InfoPluginFactory ()
{
}

InfoPluginFactory::~InfoPluginFactory ()
{
	QList<const InfoPlugin::Descriptor *> list=descriptors.values ();
	deleteList (list);
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
	if (descriptor->getId ().isNull ())
	{
		QString message=QString ("Error: invalid UUID for plugin %1").arg (descriptor->getName ());
		std::cerr << message << std::endl;
	}
	else if (descriptors.contains (descriptor->getId ()))
	{
		QString message=QString ("Error: duplicate UUID %1 for plugins %2 and %3").arg (
				descriptor->getId (),
				descriptor->getName (),
				descriptors.value (descriptor->getId ())->getName ());

		std::cerr << message << std::endl;
	}
	else
	{
		descriptors.insert (descriptor->getId (), descriptor);
	}
}

/**
 * Returns the list of all known info plugin descriptors
 *
 * @return a list of pointers to info plugin descriptors. The caller may not
 *         delete any of the pointers.
 */
const QList<const InfoPlugin::Descriptor *> InfoPluginFactory::getDescriptors ()
{
	return descriptors.values ();
}

/**
 * Finds the descriptor for a plugin with the given ID
 *
 * If no plugin with the given ID has been registered, NULL is returned. If
 * multiple plugins with the same ID have been registered, an arbitrary one of
 * them is returned.
 *
 * This method is private in order to minimize the chance of accidently
 * deleting a descriptor. #create should be used instead where applicable.
 *
 * @param id the ID of the plugin to find
 * @return a pointer to the descriptor for the plugin, or NULL
 */
const InfoPlugin::Descriptor *InfoPluginFactory::find (const QUuid &id) const
{
	return descriptors.value (id);
}

/**
 * Creates an info plugin with a given ID
 *
 * If no plugin with the given ID has been registered, NULL is returned.
 *
 * The caller takes ownership of the returned InfoPlugin instance.
 *
 * @param id the ID of the plugin to create
 * @return an InfoPlugin instance, or NULL
 * @see #find
 */
InfoPlugin *InfoPluginFactory::create (const QUuid &id) const
{
	const InfoPlugin::Descriptor *descriptor=find (id);
	if (!descriptor) return NULL;

	return descriptor->create ();
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
