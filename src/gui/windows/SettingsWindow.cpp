#include "SettingsWindow.h"

/*
server <host> (localhost)
port <num> (3306)
database <name> (startkladde)
username <name> (startkladde)
password <pass> (moobert)

debug (false)
title <text> (Startkladde Dingenskirchen)
diag_cmd <cmd> (xterm -e ./netztest &)
ort <name> (Dingenskirchen)
style <name> (-)
source <filename> (multi)
record_towpilot (true)
protect_launch_methods (false)

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
weather_dialog_title title ("Retenradar (4 Stunden)")
weather_dialog_interval interval
*/

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

SettingsWindow::~SettingsWindow()
{

}
