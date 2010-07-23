/*
 * PluginFactory.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "PluginFactory.h"

#include <iostream>

#include <QList>

#include "src/util/qList.h"
#include "src/util/qString.h"

// FIXME massive code duplication info/weather

// ******************
// ** Construction **
// ******************

PluginFactory::PluginFactory ()
{
}

PluginFactory::~PluginFactory ()
{
	QList<const InfoPlugin::Descriptor *> list=infoPluginDescriptors.values ();
	deleteList (list);
}


// ***************
// ** Singleton **
// ***************

PluginFactory *PluginFactory::instance;

/**
 * Returns the singleton instance of PluginFactory
 *
 * @return the singleton instance of PluginFactory
 */
PluginFactory &PluginFactory::getInstance ()
{
	if (!PluginFactory::instance)
		PluginFactory::instance=new PluginFactory;

	return *PluginFactory::instance;
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
void PluginFactory::addDescriptor (InfoPlugin::Descriptor *descriptor)
{
	if (descriptor->getId ().isNull ())
	{
		QString message=QString ("Error: invalid UUID for plugin %1").arg (descriptor->getName ());
		std::cerr << message << std::endl;
	}
	else if (infoPluginDescriptors.contains (descriptor->getId ()))
	{
		QString message=QString ("Error: duplicate UUID %1 for plugins %2 and %3").arg (
				descriptor->getId (),
				descriptor->getName (),
				infoPluginDescriptors.value (descriptor->getId ())->getName ());

		std::cerr << message << std::endl;
	}
	else
	{
		infoPluginDescriptors.insert (descriptor->getId (), descriptor);
	}
}

void PluginFactory::addDescriptor (WeatherPlugin::Descriptor *descriptor)
{
	if (descriptor->getId ().isNull ())
	{
		QString message=QString ("Error: invalid UUID for plugin %1").arg (descriptor->getName ());
		std::cerr << message << std::endl;
	}
	else if (weatherPluginDescriptors.contains (descriptor->getId ()))
	{
		QString message=QString ("Error: duplicate UUID %1 for plugins %2 and %3").arg (
				descriptor->getId (),
				descriptor->getName (),
				weatherPluginDescriptors.value (descriptor->getId ())->getName ());

		std::cerr << message << std::endl;
	}
	else
	{
		weatherPluginDescriptors.insert (descriptor->getId (), descriptor);
	}
}
/**
 * Returns the list of all known info plugin descriptors
 *
 * @return a list of pointers to info plugin descriptors. The caller may not
 *         delete any of the pointers.
 */
const QList<const InfoPlugin::Descriptor *> PluginFactory::getInfoPluginDescriptors ()
{
	return infoPluginDescriptors.values ();
}

const QList<const WeatherPlugin::Descriptor *> PluginFactory::getWeatherPluginDescriptors ()
{
	return weatherPluginDescriptors.values ();
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
const InfoPlugin::Descriptor *PluginFactory::findInfoPlugin (const QUuid &id) const
{
	return infoPluginDescriptors.value (id);
}

const WeatherPlugin::Descriptor *PluginFactory::findWeatherPlugin (const QUuid &id) const
{
	return weatherPluginDescriptors.value (id);
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
InfoPlugin *PluginFactory::createInfoPlugin (const QUuid &id) const
{
	const InfoPlugin::Descriptor *descriptor=findInfoPlugin (id);
	if (!descriptor) return NULL;

	return descriptor->create ();
}

WeatherPlugin *PluginFactory::createWeatherPlugin (const QUuid &id) const
{
	const WeatherPlugin::Descriptor *descriptor=findWeatherPlugin (id);
	if (!descriptor) return NULL;

	return descriptor->create ();
}


// *************************************
// ** PluginFactory::Registration **
// *************************************

/**
 * Registers an info plugin
 *
 * @param descriptor the descriptor of the plugin to register
 */
PluginFactory::InfoPluginRegistration::InfoPluginRegistration (InfoPlugin::Descriptor *descriptor)
{
	PluginFactory::getInstance ().addDescriptor (descriptor);
}

PluginFactory::WeatherPluginRegistration::WeatherPluginRegistration (WeatherPlugin::Descriptor *descriptor)
{
	PluginFactory::getInstance ().addDescriptor (descriptor);
}
