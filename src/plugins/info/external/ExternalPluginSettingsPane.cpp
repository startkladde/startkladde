#include "ExternalPluginSettingsPane.h"

#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/external/ExternalPlugin.h"

// FIXME: browsing, file resolving sunset plugin style
// FIXME: parameters (?)
// FIXME: restarting?

ExternalPluginSettingsPane::ExternalPluginSettingsPane (ExternalPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);
}

ExternalPluginSettingsPane::~ExternalPluginSettingsPane()
{

}

void ExternalPluginSettingsPane::readSettings ()
{
	ui.commandInput    ->setText    (plugin->getCommand  ());
	ui.richTextCheckbox->setChecked (plugin->getRichText ());
}

bool ExternalPluginSettingsPane::writeSettings ()
{
	plugin->setCommand  (ui.commandInput    ->text      ());
	plugin->setRichText (ui.richTextCheckbox->isChecked ());
	return true;
}
