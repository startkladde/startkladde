#ifndef INFOPLUGINFACTORY_H_
#define INFOPLUGINFACTORY_H_

#include <QString>
#include <QList>

#include "src/plugin/info/InfoPlugin.h" // required for InfoPlugin::Descriptor

#define REGISTER_INFO_PLUGIN(klass) InfoPluginFactory::Registration klass ## Descriptor (new InfoPlugin::DefaultDescriptor<klass> ());

class InfoPluginFactory
{
		// ***********
		// ** Types **
		// ***********

	public:
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
		const QList<InfoPlugin::Descriptor *> &getDescriptors ();
		const InfoPlugin::Descriptor *find (const QString &id) const;
		InfoPlugin *create (const QString &id, const QString &caption="") const;

	private:
		void addDescriptor (InfoPlugin::Descriptor *descriptor);
		QList<InfoPlugin::Descriptor *> descriptors;
};

#endif
