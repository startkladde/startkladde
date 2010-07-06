#ifndef INFOPLUGINSETTINGSPANE_H
#define INFOPLUGINSETTINGSPANE_H

#include <QtGui/QWidget>
#include "ui_InfoPluginSettingsPane.h"

#include "src/plugin/settings/PluginSettingsPane.h"

class InfoPlugin;
class PluginSettingsPane;

class InfoPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		InfoPluginSettingsPane (InfoPlugin *plugin, QWidget *parent=NULL);
		~InfoPluginSettingsPane ();

	public slots:
		virtual void readSettings ();
		virtual void writeSettings ();

	private:
		Ui::InfoPluginSettingsPaneClass ui;

		InfoPlugin *plugin;
		PluginSettingsPane *infoPluginSettingsPane;
};

#endif
