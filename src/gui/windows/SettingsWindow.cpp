/*
 * Improvements:
 *   - pluginPathList: after dragging, select the dragged item in the new
 *     position
 *   - infoPluginList: enable internal dragging
 *   - pluginPathList/infoPluginList:
 *     - contextmenu
 *     - swapping with alt+up/alt+down
 *     - when double-clicking in the empty area, add an item
 *   - reset all to default
 */
#include "SettingsWindow.h"

#include <iostream>
#include <cassert>

#include <QItemEditorFactory>
#include <QSettings>
#include <QInputDialog>
#include <QPushButton>
#include <QDebug>

#include "src/config/Settings.h"
#include "src/db/DatabaseInfo.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/info/InfoPluginFactory.h"
#include "src/plugin/ShellPluginInfo.h"
#include "src/gui/views/ReadOnlyItemDelegate.h"
#include "src/gui/views/SpinBoxCreator.h"
#include "src/gui/views/SpecialIntDelegate.h"
#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/gui/dialogs.h"

//const int columnTitle=0;
//const int columnCommand=1;
//const int columnEnabled=2;
//const int columnRichText=3;
//const int columnInterval=4;
//const int columnWarnOnDeath=5;

const int columnCaption=0;
const int columnName=1;
const int columnEnabled=2;

SettingsWindow::SettingsWindow (QWidget *parent):
	QDialog (parent),
	warned (false),
	databaseSettingsChanged (false)
{
	ui.setupUi (this);
	ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");

	ui.dbTypePane->setVisible (false);

	// Make boolean columns and some other columns read-only
	ui.infoPluginList->setItemDelegateForColumn (columnName       , new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn (columnEnabled    , new ReadOnlyItemDelegate (ui.infoPluginList));

	// Note that this label does not use wordWrap because it causes the minimum
	// size of the label not to work properly.
	ui.passwordMessageLabel->setText (ui.passwordMessageLabel->text ().arg (QSettings ().fileName ()));

	readSettings ();
	updateWidgets ();
}

SettingsWindow::~SettingsWindow()
{
	deleteList (infoPlugins);
}

void SettingsWindow::on_buttonBox_accepted ()
{
	if (allowEdit ())
	{
		writeSettings ();
		close ();
	}
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
	// Data
	ui.locationInput         ->setText    (s.location);
	ui.recordTowpilotCheckbox->setChecked (s.recordTowpilot);
	// Permissions
	ui.protectSettingsCheckbox     ->setChecked (s.protectSettings);
	ui.protectLaunchMethodsCheckbox->setChecked (s.protectLaunchMethods);
	// Diagnostics
	ui.enableDebugCheckbox->setChecked (s.enableDebug);
	ui.diagCommandInput   ->setText    (s.diagCommand);

	// *** Plugins - Info
	deleteList (infoPlugins);
	infoPlugins=s.readInfoPlugins ();

	ui.infoPluginList->clear ();
	foreach (InfoPlugin *plugin, infoPlugins)
	{
		QTreeWidgetItem *item=new QTreeWidgetItem (ui.infoPluginList);
		readItem (item, plugin);
	}
	for (int i=0; i<ui.infoPluginList->columnCount (); ++i)
		ui.infoPluginList->resizeColumnToContents (i);


	// *** Plugins - Weather
	// Weather plugin
	ui.weatherPluginBox          ->setChecked (s.weatherPluginEnabled);
	ui.weatherPluginCommandInput ->setText    (s.weatherPluginCommand );
	ui.weatherPluginHeightInput  ->setValue   (s.weatherPluginHeight  );
	ui.weatherPluginIntervalInput->setValue   (s.weatherPluginInterval);
	// Weather dialog
	ui.weatherWindowBox          ->setChecked (s.weatherWindowEnabled);
	ui.weatherWindowCommandInput ->setText    (s.weatherWindowCommand );
	ui.weatherWindowIntervalInput->setValue   (s.weatherWindowInterval);
	ui.weatherWindowTitleInput   ->setText    (s.weatherWindowTitle   );

	// *** Plugins - Paths
	ui.pluginPathList->clear ();
	foreach (const QString &pluginPath, s.pluginPaths)
		ui.pluginPathList->addItem (pluginPath);

	int n=ui.pluginPathList->count ();
	for (int i=0; i<n; ++i)
		makeItemEditable (ui.pluginPathList->item (i));

	updateWidgets ();
}

void SettingsWindow::readItem (QTreeWidgetItem *item, const InfoPlugin *plugin)
{
	// FIXME use a model (InfoPluginList)?
	item->setData       (columnCaption, Qt::DisplayRole, plugin->getCaption ());
	item->setData       (columnName,    Qt::DisplayRole, plugin->getName ());
	item->setCheckState (columnEnabled, true?Qt::Checked:Qt::Unchecked); // FIXME

	item->setFlags (item->flags () | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
}

void SettingsWindow::makeItemEditable (QListWidgetItem *item)
{
	item->setFlags (item->flags () | Qt::ItemIsEditable);
}

void SettingsWindow::writeSettings ()
{
	Settings &s=Settings::instance ();
	DatabaseInfo &info=s.databaseInfo;

	DatabaseInfo oldInfo=info;

	// *** Database
	info.server     =ui.mysqlServerInput        ->text ();
	info.defaultPort=ui.mysqlDefaultPortCheckBox->isChecked ();
	info.port       =ui.mysqlPortInput          ->value ();
	info.username   =ui.mysqlUserInput          ->text ();
	info.password   =ui.mysqlPasswordInput      ->text ();
	info.database   =ui.mysqlDatabaseInput      ->text ();

	// *** Settings
	// Data
	s.location      =ui.locationInput         ->text ();
	s.recordTowpilot=ui.recordTowpilotCheckbox->isChecked ();
	// Permissions
	s.protectSettings     =ui.protectSettingsCheckbox     ->isChecked ();
	s.protectLaunchMethods=ui.protectLaunchMethodsCheckbox->isChecked ();
	// Diagnostics
	s.enableDebug=ui.enableDebugCheckbox->isChecked ();
	s.diagCommand=ui.diagCommandInput   ->text ();

	// *** Plugins - Info
	int numInfoPlugins=infoPlugins.size ();
	assert (numInfoPlugins==ui.infoPluginList->topLevelItemCount ());
	for (int i=0; i<numInfoPlugins; ++i)
	{
		QTreeWidgetItem &item=*ui.infoPluginList->topLevelItem (i);
		infoPlugins[i]->setCaption (item.data (columnCaption, Qt::DisplayRole).toString ());
	}
	s.writeInfoPlugins (infoPlugins);


	// *** Plugins - Weather
	// Weather plugin
	s.weatherPluginEnabled =ui.weatherPluginBox          ->isChecked ();
	s.weatherPluginCommand =ui.weatherPluginCommandInput ->text ();
	s.weatherPluginHeight  =ui.weatherPluginHeightInput  ->value ();
	s.weatherPluginInterval=ui.weatherPluginIntervalInput->value ();
	// Weather dialog
	s.weatherWindowEnabled =ui.weatherWindowBox          ->isChecked ();
	s.weatherWindowCommand =ui.weatherWindowCommandInput ->text ();
	s.weatherWindowInterval=ui.weatherWindowIntervalInput->value ();
	s.weatherWindowTitle   =ui.weatherWindowTitleInput   ->text ();

	// *** Plugins - Paths
	s.pluginPaths.clear ();
	int numPluginPaths=ui.pluginPathList->count ();
	for (int i=0; i<numPluginPaths; ++i)
		s.pluginPaths << ui.pluginPathList->item (i)->text ();

	s.save ();

	databaseSettingsChanged=oldInfo.different (info);
}

void SettingsWindow::updateWidgets ()
{
	ui.mysqlPortInput->setEnabled (!ui.mysqlDefaultPortCheckBox->isChecked ());
}

void SettingsWindow::on_addPluginPathButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	list->addItem ("");
	makeItemEditable (list->item (list->count ()-1));
	list->setCurrentRow (list->count ()-1);
	list->editItem (list->item (list->count ()-1));
}

void SettingsWindow::on_removePluginPathButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	delete list->takeItem (row);
	if (row>=list->count ()) --row;
	if (row>=0) list->setCurrentRow (row);
}

void SettingsWindow::on_pluginPathUpButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	if (row==0) return;

	list->insertItem (row-1, list->takeItem (row));
	list->setCurrentRow (row-1);
}

void SettingsWindow::on_pluginPathDownButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	if (row==list->count ()-1) return;

	list->insertItem (row+1, list->takeItem (row));
	list->setCurrentRow (row+1);
}





void SettingsWindow::on_addInfoPluginButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	// FIXME
	InfoPlugin *plugin=InfoPluginFactory::getInstance ().find ("test")->create ();
	infoPlugins.append (plugin);

	QTreeWidgetItem *item=new QTreeWidgetItem (list);
	readItem (item, plugin);

	list->setCurrentItem (item);
	list->editItem (item, 0);
}

void SettingsWindow::on_removeInfoPluginButton_clicked ()
{
	warnEdit ();
 	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;

	delete list->takeTopLevelItem (row);
	delete infoPlugins.takeAt (row);

	if (row>=list->topLevelItemCount ()) --row;
	if (row>=0) list->setCurrentItem (list->topLevelItem (row));
}

void SettingsWindow::on_infoPluginUpButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;
	if (row==0) return;

	list->insertTopLevelItem (row-1, list->takeTopLevelItem (row));
	infoPlugins.insert (row-1, infoPlugins.takeAt (row));

	list->setCurrentItem (list->topLevelItem (row-1));
}

void SettingsWindow::on_infoPluginDownButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;
	if (row==list->topLevelItemCount ()-1) return;

	list->insertTopLevelItem (row+1, list->takeTopLevelItem (row));
	infoPlugins.insert (row+1, infoPlugins.takeAt (row));

	list->setCurrentItem (list->topLevelItem (row+1));
}

bool SettingsWindow::allowEdit ()
{
	QString message;
	QString requiredPassword;

	QString oldPassword=Settings::instance ().databaseInfo.password;
	QString newPassword=ui.mysqlPasswordInput->text ();
	bool passwordChanged=(newPassword!=oldPassword);

	if (Settings::instance ().protectSettings)
	{
		message=utf8 ("Zum Speichern der Einstellungen ist das %1Datenbankpasswort\n"
			"erforderlich.").arg (passwordChanged?"(alte) ":"");
		requiredPassword=oldPassword;
	}
	else if (ui.protectSettingsCheckbox->isChecked ())
	{
		message=utf8 (
			"Der Passwortschutz der Einstellungen wird aktiviert. Dazu ist das\n"
			"%1Datenbankpasswort erforderlich. Fall der Schutz nicht aktiviert\n"
			"werden soll, kann jetzt abgebrochen und die entsprechende Option\n"
			"deaktiviert werden."
			).arg (passwordChanged?"(neue) ":"");
		requiredPassword=newPassword;
	}
	else
		return true;


	while (true)
	{
		bool ok=false;
		QString enteredPassword=QInputDialog::getText (this, "Passwort erforderlich",
			utf8 ("%1 Bitte Passwort eingeben:").arg (message), QLineEdit::Password, QString (), &ok);

		// Canceled
		if (!ok) return false;

		if (enteredPassword==requiredPassword)
			return true;

		message="Das eingegebene Passwort ist nicht korrekt.";
	}
}

void SettingsWindow::warnEdit ()
{
	if (!Settings::instance ().protectSettings) return;
	if (warned) return;

	showWarning (utf8 ("Einstellungen geschützt"), utf8 (
		"Achtung: Die Einstellungen sind geschützt. Die Einstellungen\n"
		"können geändert werden, aber zum Speichern ist das\n"
		"Datenbankpasswort erforderlich."), this);

	warned=true;
}
