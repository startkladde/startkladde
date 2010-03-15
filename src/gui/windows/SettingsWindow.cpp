#include "SettingsWindow.h"

#include "src/config/Settings.h"
#include "src/db/DatabaseInfo.h"

/*
diag_cmd <cmd> (xterm -e ./netztest &)

plugin_path path (.) (multi)
shell_plugin <title>, <command>, <interval not 0> [, warn_on_death] [, rich_text (see doc/plugins)]
    shell_plugin Sunset:, sunset_time sunsets, -1
    shell_plugin Zeit bis sunset:, sunset_countdown sunsets, 60, rich_text
    shell_plugin Wind:, wind /tmp/wind, 1, warn_on_death
	shell_plugin System:, /bin/uname -a, -1

weather_plugin <command> (plugins/weather/regenradar_wetter.com)
weather_height <height> (200, Videos werden abgeschnitten)
weather_interval interval (600)
weather_dialog_plugin command ("")
weather_dialog_title title ("Regenradar (4 Stunden)")
weather_dialog_interval interval

style <name> (-)
source <filename> (multi)
*/

SettingsWindow::SettingsWindow (QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);

	ui.dbTypePane->setVisible (false);

	readSettings ();
	updateWidgets ();
}

SettingsWindow::~SettingsWindow()
{

}

void SettingsWindow::on_buttonBox_accepted ()
{
	writeSettings ();
	close ();
}

void SettingsWindow::readSettings ()
{
	Settings &s=Settings::instance ();
	DatabaseInfo &info=s.databaseInfo;

	// *** Database
	ui.mysqlServerInput        ->setText    (info.server);
	ui.mysqlDefaultPortCheckBox->setChecked (info.defaultPort); // TODO enable port
	ui.mysqlPortInput          ->setValue   (info.port);
	ui.mysqlUserInput          ->setText    (info.username);
	ui.mysqlPasswordInput      ->setText    (info.password);
	ui.mysqlDatabaseInput      ->setText    (info.database);

	// *** Settings
	ui.locationInput         ->setText    (s.location);
	ui.recordTowpilotCheckbox->setChecked (s.recordTowpilot);

	updateWidgets ();
}

void SettingsWindow::writeSettings ()
{
	Settings &s=Settings::instance ();
	DatabaseInfo &info=s.databaseInfo;

	// *** Database
	info.server     =ui.mysqlServerInput        ->text ();
	info.defaultPort=ui.mysqlDefaultPortCheckBox->isChecked ();
	info.port       =ui.mysqlPortInput          ->value ();
	info.username   =ui.mysqlUserInput          ->text ();
	info.password   =ui.mysqlPasswordInput      ->text ();
	info.password   =ui.mysqlDatabaseInput      ->text ();

	// *** Settings
	s.location      =ui.locationInput         ->text ();
	s.recordTowpilot=ui.recordTowpilotCheckbox->isChecked ();

	s.save ();
}

void SettingsWindow::updateWidgets ()
{
	ui.mysqlPortInput->setEnabled (!ui.mysqlDefaultPortCheckBox->isChecked ());
}
