#ifndef ExternalPluginSETTINGSPANE_H
#define ExternalPluginSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_ExternalPluginSettingsPane.h"

#include <QtGui/QWidget>

class ExternalPlugin;

class ExternalPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		ExternalPluginSettingsPane (ExternalPlugin *plugin, QWidget *parent=NULL);
		~ExternalPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	private:
		Ui::ExternalPluginSettingsPaneClass ui;

		ExternalPlugin *plugin;
};

#endif
