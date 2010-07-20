#ifndef INFOPLUGINFACTORY_H_
#define INFOPLUGINFACTORY_H_

#include <QString>
#include <QList>
#include <QHash>

#include "src/plugin/info/InfoPlugin.h" // required for InfoPlugin::Descriptor

/**
 * Registers the given info plugin (implementation of InfoPlugin) by creating
 * an instance of the InfoPluginFactory::Registration class.
 *
 * Use this macro in the cpp file of the plugin implementation.
 */
#define REGISTER_INFO_PLUGIN(klass) InfoPluginFactory::Registration klass ## Descriptor \
	(new InfoPlugin::DefaultDescriptor<klass> ());

/**
 * A factory for creating InfoPlugin instances
 *
 * This class is a singleton.
 *
 * Info plugins are identified by their ID.
 */
class InfoPluginFactory
{
		// ***********
		// ** Types **
		// ***********

	public:
		/**
		 * RAAI registration of info plugins
		 *
		 * This allow registration of info plugins by creating a static
		 * instance in a compilation unit.
		 *
		 * This class is intended to be used via the REGISTER_INFO_PLUGIN
		 * macro.
		 */
		class Registration
		{
			public:
				Registration (InfoPlugin::Descriptor *descriptor);
		};

		// ******************
		// ** Construction **
		// ******************

	public: virtual ~InfoPluginFactory ();
	private: InfoPluginFactory ();

		// ***************
		// ** Singleton **
		// ***************

	public: static InfoPluginFactory &getInstance ();
	private: static InfoPluginFactory *instance;


		// *************
		// ** Factory **
		// *************

	public:
		const QList<const InfoPlugin::Descriptor *> getDescriptors ();
		InfoPlugin *create (const QString &id) const;

	private:
		void addDescriptor (InfoPlugin::Descriptor *descriptor);
		const InfoPlugin::Descriptor *find (const QString &id) const;

		QHash<QString, const InfoPlugin::Descriptor *> descriptors;
};

#endif
