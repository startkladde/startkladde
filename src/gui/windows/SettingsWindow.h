#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QtGui/QDialog>

#include "ui_SettingsWindow.h"

class SettingsWindow: public QDialog
{
    Q_OBJECT

	public:
		SettingsWindow (QWidget *parent=0);
		~SettingsWindow ();

	protected:
		void readSettings ();
		void writeSettings ();

		void makeItemEditable (QListWidgetItem *item);

	private slots:
		void on_mysqlDefaultPortCheckBox_toggled () { updateWidgets (); }

		void on_addPluginPathButton_clicked ();
		void on_removePluginPathButton_clicked ();
		void on_pluginPathUpButton_clicked ();
		void on_pluginPathDownButton_clicked ();

		void updateWidgets ();

		void on_buttonBox_accepted ();

	private:
		Ui::SettingsWindowClass ui;
};

#endif
