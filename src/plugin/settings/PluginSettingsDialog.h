#ifndef PLUGINSETTINGSDIALOG_H
#define PLUGINSETTINGSDIALOG_H

#include <QtGui/QDialog>
#include "ui_PluginSettingsDialog.h"

class Plugin;
class QWidget;
class PluginSettingsPane;

class PluginSettingsDialog : public QDialog
{
		Q_OBJECT

	public:
		PluginSettingsDialog (Plugin *plugin, QWidget *parent=NULL);
		~PluginSettingsDialog ();

		static int invoke (Plugin *plugin, QWidget *parent=NULL);

	private slots:
		void on_buttonBox_accepted ();

	private:
		Ui::PluginSettingsDialogClass ui;

		PluginSettingsPane *settingsPane;
};

#endif
