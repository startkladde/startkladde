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

#include <QItemEditorFactory>
#include <QSettings>
#include <QInputDialog>

#include "src/config/Settings.h"
#include "src/db/DatabaseInfo.h"
#include "src/plugins/ShellPluginInfo.h"
#include "src/gui/views/ReadOnlyItemDelegate.h"
#include "src/gui/views/SpinBoxCreator.h"
#include "src/gui/views/SpecialIntDelegate.h"
#include "src/util/qString.h"
#include "src/gui/dialogs.h"

SettingsWindow::SettingsWindow (QWidget *parent):
	QDialog (parent),
	warned (false)
{
	ui.setupUi (this);

	ui.dbTypePane->setVisible (false);

	// Make boolean columns read-only
	ui.infoPluginList->setItemDelegateForColumn (2, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn (4, new ReadOnlyItemDelegate (ui.infoPluginList));

	// Setup the integer colum
	QStyledItemDelegate *intervalDelegate=new SpecialIntDelegate (0, "Nicht neu starten", " s", ui.infoPluginList);
	SpinBoxCreator *spinBoxCreator=new SpinBoxCreator (0, "Nicht neu starten", " s");
	QItemEditorFactory *factory=new QItemEditorFactory;
	factory->registerEditor (QVariant::Int, spinBoxCreator);
	intervalDelegate->setItemEditorFactory (factory);
	ui.infoPluginList->setItemDelegateForColumn (3, intervalDelegate);

	ui.passwordMessageLabel->setText (ui.passwordMessageLabel->text ().arg (QSettings ().fileName ()));

	readSettings ();
	updateWidgets ();
}

SettingsWindow::~SettingsWindow()
{

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
	ui.infoPluginList->clear ();
	foreach (const ShellPluginInfo &plugin, s.infoPlugins)
	{
		QTreeWidgetItem *item=new QTreeWidgetItem (ui.infoPluginList);
		readItem (item, plugin);
	}
	for (int i=0; i<ui.infoPluginList->columnCount (); ++i)
		ui.infoPluginList->resizeColumnToContents (i);


	// *** Plugins - Weather
	// Weather plugin
	ui.weatherPluginCommandInput ->setText  (s.weatherPluginCommand );
	ui.weatherPluginHeightInput  ->setValue (s.weatherPluginHeight  );
	ui.weatherPluginIntervalInput->setValue (s.weatherPluginInterval);
	// Weather dialog
	ui.weatherWindowCommandInput ->setText  (s.weatherWindowCommand );
	ui.weatherWindowIntervalInput->setValue (s.weatherWindowInterval);
	ui.weatherWindowTitleInput   ->setText  (s.weatherWindowTitle   );

	// *** Plugins - Paths
	ui.pluginPathList->clear ();
	foreach (const QString &pluginPath, s.pluginPaths)
		ui.pluginPathList->addItem (pluginPath);

	int n=ui.pluginPathList->count ();
	for (int i=0; i<n; ++i)
		makeItemEditable (ui.pluginPathList->item (i));

	updateWidgets ();
}

void SettingsWindow::readItem (QTreeWidgetItem *item, const ShellPluginInfo &plugin)
{
	item->setData       (0, Qt::DisplayRole, plugin.caption);
	item->setData       (1, Qt::DisplayRole, plugin.command);
	item->setCheckState (2, plugin.richText?Qt::Checked:Qt::Unchecked);
	item->setData       (3, Qt::DisplayRole, plugin.restartInterval);
	item->setCheckState (4, plugin.warnOnDeath?Qt::Checked:Qt::Unchecked);

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
	s.infoPlugins.clear ();
	int numInfoPlugins=ui.infoPluginList->topLevelItemCount ();
	for (int i=0; i<numInfoPlugins; ++i)
	{
		QTreeWidgetItem &item=*ui.infoPluginList->topLevelItem (i);
		s.infoPlugins << ShellPluginInfo (
			item.data (0, Qt::DisplayRole).toString (), // caption
			item.data (1, Qt::DisplayRole).toString (), // command
			item.checkState (2)==Qt::Checked,           // richText
			item.data (3, Qt::DisplayRole).toInt    (), // restartInterval
			item.checkState (4)==Qt::Checked            // warnOnDeath
			);
	}

	// *** Plugins - Weather
	// Weather plugin
	s.weatherPluginCommand =ui.weatherPluginCommandInput ->text ();
	s.weatherPluginHeight  =ui.weatherPluginHeightInput  ->value ();
	s.weatherPluginInterval=ui.weatherPluginIntervalInput->value ();
	// Weather dialog
	s.weatherWindowCommand =ui.weatherWindowCommandInput ->text ();
	s.weatherWindowInterval=ui.weatherWindowIntervalInput->value ();
	s.weatherWindowTitle   =ui.weatherWindowTitleInput   ->text ();

	// *** Plugins - Paths
	s.pluginPaths.clear ();
	int numPluginPaths=ui.pluginPathList->count ();
	for (int i=0; i<numPluginPaths; ++i)
		s.pluginPaths << ui.pluginPathList->item (i)->text ();

	s.save ();
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

	QTreeWidgetItem *item=new QTreeWidgetItem (list);
	readItem (item, ShellPluginInfo ());

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
	list->setCurrentItem (list->topLevelItem (row+1));
}

bool SettingsWindow::allowEdit ()
{
	QString message;

	if (Settings::instance ().protectSettings)
		message=utf8 ("Zum Speichern der Einstellungen ist das Datenbankpasswort\n"
			"erforderlich.");
	else if (ui.protectSettingsCheckbox->isChecked ())
		message=utf8 ("Der Passwortschutz der Einstellungen wird aktiviert. Dazu\n"
			"ist das Datenbankpasswort erforderlich. Fall der Schutz nicht aktiviert\n"
			"werden soll, kann jetzt abgebrochen und die entsprechende Option\n"
			"deaktiviert werden."
			);
	else
		return true;


	while (true)
	{
		bool ok=false;
		QString enteredPassword=QInputDialog::getText (this, "Passwort erforderlich",
			utf8 ("%1 Bitte Passwort eingeben:").arg (message), QLineEdit::Password, QString (), &ok);

		// Canceled
		if (!ok) return false;

		if (enteredPassword==Settings::instance ().databaseInfo.password)
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
