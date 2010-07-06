#include "InfoPluginSettingsPane.h"

#include "src/plugin/info/InfoPlugin.h"

InfoPluginSettingsPane::InfoPluginSettingsPane (InfoPlugin *plugin, QWidget *parent):
    PluginSettingsPane (parent),
    plugin (plugin)
{
	ui.setupUi (this);

	infoPluginSettingsPane=plugin->infoPluginCreateSettingsPane (this);
	ui.infoPluginSettingsPane->layout ()->addWidget (infoPluginSettingsPane);

	ui.captionInput->setFocus ();
}

InfoPluginSettingsPane::~InfoPluginSettingsPane()
{

}

void InfoPluginSettingsPane::readSettings ()
{

	ui.captionInput->setText (plugin->getCaption ());
	infoPluginSettingsPane->readSettings ();
	ui.captionInput->setFocus ();
}

void InfoPluginSettingsPane::writeSettings ()
{
	plugin->setCaption (ui.captionInput->text ());
	infoPluginSettingsPane->writeSettings ();
}
