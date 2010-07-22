#ifndef EXTERNALPLUGIN_H_
#define EXTERNALPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

class ExternalPlugin: public InfoPlugin
{
		SK_PLUGIN

	public:
		ExternalPlugin (const QString &caption=QString (), bool enabled=true, const QString &command="", bool richText=false);
		virtual ~ExternalPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, Command , command );
		value_accessor (bool   , RichText, richText);

		virtual QString configText () const;

	private:
		QString command;
		bool richText;
};

#endif
