#ifndef INFOPLUGINFACTORY_H_
#define INFOPLUGINFACTORY_H_

#include <QString>
#include <QList>

#include "src/plugin/info/InfoPlugin.h" // required for InfoPlugin::Descriptor

#define REGISTER_INFO_PLUGIN(klass) InfoPluginFactory::Registration klass ## Descriptor (new InfoPlugin::DefaultDescriptor<klass> ());

class InfoPluginFactory
{
	public:
		// ***********
		// ** Types **
		// ***********

		class Registration
		{
			public:
				Registration (InfoPlugin::Descriptor *descriptor);
		};

		// ******************
		// ** Construction **
		// ******************

	public:
		virtual ~InfoPluginFactory ();

	private:
		InfoPluginFactory ();

		// ***************
		// ** Singleton **
		// ***************

	public:
		static InfoPluginFactory &getInstance ();

	private:
		static InfoPluginFactory *instance;


		// *****************
		// ** Descriptors **
		// *****************

	public:
		const QList<InfoPlugin::Descriptor *> &getDescriptors ();

	private:
		void addDescriptor (InfoPlugin::Descriptor *descriptor);
		QList<InfoPlugin::Descriptor *> descriptors;
};

#endif
