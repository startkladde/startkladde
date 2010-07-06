#include "InfoPluginSettingsPane.h"

#include "src/plugin/info/InfoPlugin.h"

/**
 * Creates an InfoPluginSettingsPane instance for the given info plugin
 *
 * @param plugin the InfoPlugin instance to be configured
 * @param parent the parent widget
 */
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

/**
 * Reads the common settings from the plugin and sets up the input fields
 * accordingly. Calls readSettings on the wrapped  PluginSettingsPane.
 *
 * @see PluginSettingsPane::readSettings
 */
void InfoPluginSettingsPane::readSettings ()
{

	ui.captionInput->setText (plugin->getCaption ());
	infoPluginSettingsPane->readSettings ();
	ui.captionInput->setFocus ();
}

/**
 * Writes the common settings from the plugin and sets up the input fields
 * accordingly, and calls readSettings on the wrapped PluginSettingsPane.
 *
 * @see PluginSettingsPane::readSettings
 */
bool InfoPluginSettingsPane::writeSettings ()
{
	if (!infoPluginSettingsPane->writeSettings ()) return false;

	plugin->setCaption (ui.captionInput->text ());

	return true;
}
