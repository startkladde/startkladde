#include "MetarPluginSettingsPane.h"

#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/metar/MetarPlugin.h"

MetarPluginSettingsPane::MetarPluginSettingsPane (MetarPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);

	ui.airportInput->setMinimumSize (100, 0);
}

MetarPluginSettingsPane::~MetarPluginSettingsPane()
{

}

void MetarPluginSettingsPane::readSettings ()
{
	ui.airportInput         ->setText  (plugin->getAirport         ());
	ui.refreshIntervalInput ->setValue (plugin->getRefreshInterval ());
}

bool MetarPluginSettingsPane::writeSettings ()
{
	plugin->setAirport         (ui.airportInput        ->text  ());
	plugin->setRefreshInterval (ui.refreshIntervalInput->value ());
	return true;
}
