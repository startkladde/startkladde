#ifndef PLUGINFACTORY_H_
#define PLUGINFACTORY_H_

#include <QString>
#include <QList>
#include <QHash>
#include <QUuid>

#include "src/plugin/info/InfoPlugin.h" // required for InfoPlugin::Descriptor
#include "src/plugin/weather/WeatherPlugin.h" // required for WeatherPlugin::Descriptor

/**
 * Registers the given info plugin (implementation of InfoPlugin) by creating
 * an instance of the PluginFactory::Registration class.
 *
 * Use this macro in the cpp file of the plugin implementation.
 */
#define REGISTER_INFO_PLUGIN(klass) PluginFactory::InfoPluginRegistration klass ## Descriptor \
	(new InfoPlugin::DefaultDescriptor<klass> ());

#define REGISTER_WEATHER_PLUGIN(klass) PluginFactory::WeatherPluginRegistration klass ## Descriptor \
	(new WeatherPlugin::DefaultDescriptor<klass> ());


/**
 * A factory for creating Plugin instances
 *
 * This class is a singleton.
 *
 * Plugins are identified by their ID.
 */
class PluginFactory
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
		class InfoPluginRegistration
		{
			public:
				InfoPluginRegistration (InfoPlugin::Descriptor *descriptor);
		};

		class WeatherPluginRegistration
		{
			public:
				WeatherPluginRegistration (WeatherPlugin::Descriptor *descriptor);
		};

		// ******************
		// ** Construction **
		// ******************

	public: virtual ~PluginFactory ();
	private: PluginFactory ();

		// ***************
		// ** Singleton **
		// ***************

	public: static PluginFactory &getInstance ();
	private: static PluginFactory *instance;


		// *************
		// ** Factory **
		// *************

	public:
		const QList<const    InfoPlugin::Descriptor *> getInfoPluginDescriptors ();
		const QList<const WeatherPlugin::Descriptor *> getWeatherPluginDescriptors ();

		InfoPlugin    *createInfoPlugin    (const QUuid &id) const;
		WeatherPlugin *createWeatherPlugin (const QUuid &id) const;

	private:
		void addDescriptor (   InfoPlugin::Descriptor *descriptor);
		void addDescriptor (WeatherPlugin::Descriptor *descriptor);

		const    InfoPlugin::Descriptor *findInfoPlugin    (const QUuid &id) const;
		const WeatherPlugin::Descriptor *findWeatherPlugin (const QUuid &id) const;

		QHash<QUuid, const InfoPlugin   ::Descriptor *>    infoPluginDescriptors;
		QHash<QUuid, const WeatherPlugin::Descriptor *> weatherPluginDescriptors;
};

#endif
