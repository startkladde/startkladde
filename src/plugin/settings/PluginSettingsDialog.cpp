#include "PluginSettingsDialog.h"

#include <QPushButton>

#include "src/plugin/Plugin.h"
#include "src/plugin/settings/PluginSettingsPane.h"

PluginSettingsDialog::PluginSettingsDialog (Plugin *plugin, QWidget *parent):
	QDialog (parent)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");

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

int PluginSettingsDialog::invoke (Plugin *plugin, QWidget *parent)
{
	PluginSettingsDialog *dialog=new PluginSettingsDialog (plugin, parent);
	dialog->setModal (true);
	int result=dialog->exec ();
	delete dialog;
	return result;
}
