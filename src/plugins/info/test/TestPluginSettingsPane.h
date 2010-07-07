#ifndef TESTPLUGINSETTINGSPANE_H
#define TESTPLUGINSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_TestPluginSettingsPane.h"

#include "src/plugins/info/test/TestPlugin.h"

#include <QtGui/QWidget>

class TestPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		TestPluginSettingsPane (TestPlugin *plugin, QWidget *parent=NULL);
		~TestPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	private:
		Ui::TestPluginSettingsPaneClass ui;

		TestPlugin *plugin;
};

#endif
