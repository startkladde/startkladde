#include "TestPluginSettingsPane.h"

TestPluginSettingsPane::TestPluginSettingsPane (TestPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);
}

TestPluginSettingsPane::~TestPluginSettingsPane()
{

}

void TestPluginSettingsPane::readSettings ()
{
	ui.greetingNameInput->setText    (plugin->getGreetingName ());
	ui.richTextCheckbox ->setChecked (plugin->getRichText     ());
}

void TestPluginSettingsPane::writeSettings ()
{
	plugin->setGreetingName (ui.greetingNameInput->text      ());
	plugin->setRichText     (ui.richTextCheckbox ->isChecked ());
}
