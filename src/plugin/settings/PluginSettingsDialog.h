#ifndef PLUGINSETTINGSDIALOG_H
#define PLUGINSETTINGSDIALOG_H

#include <QtGui/QDialog>
#include "ui_PluginSettingsDialog.h"

#include "src/accessor.h"

class Plugin;
class QWidget;
class PluginSettingsPane;
class SettingsWindow;

/**
 * A dialog which allows editing the settings of a plugin by means of its
 * PluginSettingsPane
 */
class PluginSettingsDialog : public QDialog
{
		Q_OBJECT

	public:
		PluginSettingsDialog (Plugin *plugin, QWidget *parent=NULL, SettingsWindow *settingsWindow=NULL);
		~PluginSettingsDialog ();

		static int invoke (Plugin *plugin, QWidget *parent=NULL, SettingsWindow *settingsWindow=NULL);

	private slots:
		void on_buttonBox_accepted ();

	private:
		Ui::PluginSettingsDialogClass ui;

		PluginSettingsPane *settingsPane;
};

#endif
