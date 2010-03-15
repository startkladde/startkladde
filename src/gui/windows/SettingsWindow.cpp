/*
 * Improvements:
 *   - pluginPathList: after dragging, select the dragged item in the new
 *     position
 *   - pluginPathList: when double-clicking in the empty area, add an item
 *   - pluginPathList: context menu
 */
#include "SettingsWindow.h"

#include <iostream>

#include <QStandardItemModel> //remove?

#include "src/config/Settings.h"
#include "src/db/DatabaseInfo.h"

#include "src/util/qString.h" //remove

/*
plugin_path path (.) (multi)
shell_plugin <title>, <command>, <interval not 0> [, warn_on_death] [, rich_text (see doc/plugins)]
    shell_plugin Sunset:, sunset_time sunsets, -1
    shell_plugin Zeit bis sunset:, sunset_countdown sunsets, 60, rich_text
    shell_plugin Wind:, wind /tmp/wind, 1, warn_on_death
	shell_plugin System:, /bin/uname -a, -1
*/


SettingsWindow::SettingsWindow (QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);

	ui.dbTypePane->setVisible (false);

	// QTreeWidget
	// Has different editors, depending on data type
	// Problem: cannot set the checkbox column to not editable
	QTreeWidgetItem *item=new QTreeWidgetItem ((QTreeWidget *)NULL, QStringList () << "foo" << "bar");
	item->setData (1, Qt::DisplayRole, 20);
	item->setData (1, Qt::DisplayRole, QVariant ());
	item->setCheckState (2, Qt::Checked);
//	item->setFlags (item->flags () | Qt::ItemIsEditable/* | Qt::ItemIsUserCheckable*/);
	item->setFlags (item->flags () &~ Qt::ItemIsEditable);
	ui.infoPluginList->addTopLevelItem (item);


	// QTreeView with QStandardItemModel
	// Has different editors, depending on data type
	// Problem: is either not editable (and not checkable) or can still edit
	// the value beside the checkbox
	QStandardItemModel *model=new QStandardItemModel (4, 4); // leak leak leak
	QStandardItem *xitem=NULL;

	xitem=new QStandardItem ("moo");
	xitem->setData ("hasd", Qt::DisplayRole);
	model->setItem (0, 0, xitem);

	xitem=new QStandardItem ("moo");
	xitem->setData (20, Qt::DisplayRole);
	model->setItem (0, 1, xitem);

	xitem=new QStandardItem ("moo");
//	xitem->setData (QVariant (), Qt::DisplayRole);
	xitem->setFlags ((xitem->flags () | Qt::ItemIsUserCheckable) /*& Qt::ItemIsEditable*/);
//	xitem->setCheckState (Qt::Checked);
	xitem->setData(Qt::Checked, Qt::CheckStateRole);
	model->setItem (0, 1, xitem);

	// http://www.qtforum.de/forum/viewtopic.php?t=7425&sid=770807a79253bf7914fe994b4d6ce5c2

//	 for (int row = 0; row < 4; ++row) {
//	     for (int column = 0; column < 4; ++column) {
//	         QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
//	         model->setItem(row, column, item);
//	     }
//	 }
	 ui.infoPluginListView->setModel (model);

		// Try this:
	//	QItemEditorFactory *factory = new QItemEditorFactory;
	//	QItemEditorCreatorBase *nullEditorCreator=new QStandardItemEditorCreator<ColorListEditor> ();
	//	factory->registerEditor(QVariant::Color, colorListCreator);
	//	QItemEditorFactory::setDefaultFactory(factory);
	//	http://doc.trolltech.com/4.3/itemviews-coloreditorfactory.html

	 // e. g. don't use the checkbox at all, but create a drop down box with
	 // proper yes/no entries instead (maybe use QItemEditorCreator instead of StandardItemEditorCreator)
	 // maybe use a QTreeWidget instead of a QTreeView in this case

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
	// Data
	ui.locationInput         ->setText    (s.location);
	ui.recordTowpilotCheckbox->setChecked (s.recordTowpilot);
	// Permissions
	ui.protectSettingsCheckbox     ->setChecked (s.protectSettings);
	ui.protectLaunchMethodsCheckbox->setChecked (s.protectLaunchMethods);
	// Diagnostics
	ui.enableDebugCheckbox->setChecked (s.enableDebug);
	ui.diagCommandInput   ->setText    (s.diagCommand);

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
	info.password   =ui.mysqlDatabaseInput      ->text ();

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
	int n=ui.pluginPathList->count ();
	for (int i=0; i<n; ++i)
		s.pluginPaths << ui.pluginPathList->item (i)->text ();

	s.save ();
}

void SettingsWindow::updateWidgets ()
{
	ui.mysqlPortInput->setEnabled (!ui.mysqlDefaultPortCheckBox->isChecked ());
}

void SettingsWindow::on_addPluginPathButton_clicked ()
{
	QListWidget *list=ui.pluginPathList;

	list->addItem ("");
	makeItemEditable (list->item (list->count ()-1));
	list->setCurrentRow (list->count ()-1);
	list->editItem (list->item (list->count ()-1));
}

void SettingsWindow::on_removePluginPathButton_clicked ()
{
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	delete list->takeItem (row);
	if (row>=list->count ()) --row;
	if (row>=0) list->setCurrentRow (row);
}

void SettingsWindow::on_pluginPathUpButton_clicked ()
{
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	if (row==0) return;

	list->insertItem (row-1, list->takeItem (row));
	list->setCurrentRow (row-1);
}

void SettingsWindow::on_pluginPathDownButton_clicked ()
{
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	if (row==list->count ()-1) return;

	list->insertItem (row+1, list->takeItem (row));
	list->setCurrentRow (row+1);
}
