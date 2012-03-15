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
	ui.airportInput         ->setText  (plugin->getAirport         ()   );
	ui.refreshIntervalInput ->setValue (plugin->getRefreshInterval ()/60);
}

bool MetarPluginSettingsPane::writeSettings ()
{
	plugin->setAirport         (ui.airportInput        ->text  ()   );
	plugin->setRefreshInterval (ui.refreshIntervalInput->value ()*60);
	return true;
}

void MetarPluginSettingsPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
		adjustSize ();
	}
	else
		QWidget::changeEvent (event);
}
