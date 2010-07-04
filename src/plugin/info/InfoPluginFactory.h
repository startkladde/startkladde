#ifndef INFOPLUGINFACTORY_H_
#define INFOPLUGINFACTORY_H_

#include <QString>
#include <QList>

#include "src/plugin/info/InfoPlugin.h" // required for InfoPlugin::Descriptor

#define REGISTER_INFO_PLUGIN(klass) InfoPluginFactory::Registration klass ## Descriptor (new InfoPlugin::DefaultDescriptor<klass> ());

class InfoPluginFactory
{
	public:
		class Registration
		{
			public:
				Registration (InfoPlugin::Descriptor *descriptor);
		};

		virtual ~InfoPluginFactory ();

		static InfoPluginFactory &getInstance ();

	private:
		InfoPluginFactory ();
		static InfoPluginFactory *instance;

		void addDescriptor (InfoPlugin::Descriptor *descriptor);
		QList<InfoPlugin::Descriptor *> descriptors;
};

#endif
