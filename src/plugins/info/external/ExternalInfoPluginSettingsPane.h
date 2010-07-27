#ifndef EXTERNALINFOPLUGINSETTINGSPANE_H
#define EXTERNALINFOPLUGINSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_ExternalInfoPluginSettingsPane.h"

#include <QtGui/QWidget>

class ExternalInfoPlugin;

class ExternalInfoPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		ExternalInfoPluginSettingsPane (ExternalInfoPlugin *plugin, QWidget *parent=NULL);
		~ExternalInfoPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	private slots:
		virtual void on_browseButton_clicked ();

	private:
		Ui::ExternalInfoPluginSettingsPaneClass ui;

		ExternalInfoPlugin *plugin;
};

#endif
