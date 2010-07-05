#include "PluginSettingsDialog.h"

#include "src/plugin/Plugin.h"
#include "src/plugin/settings/PluginSettingsPane.h"

PluginSettingsDialog::PluginSettingsDialog (Plugin *plugin, QWidget *parent):
	QDialog (parent)
{
	ui.setupUi(this);

	settingsPane=plugin->createSettingsPane (ui.pluginSettingsPane);
	ui.pluginSettingsPane->layout ()->addWidget (settingsPane);
	settingsPane->readSettings ();
}

PluginSettingsDialog::~PluginSettingsDialog()
{

}

void PluginSettingsDialog::on_buttonBox_accepted ()
{
	settingsPane->writeSettings ();
	accept ();
}
