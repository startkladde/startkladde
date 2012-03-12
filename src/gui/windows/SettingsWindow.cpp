/*
 * Improvements:
 *   - for the plugins, we should use a model and QTreeView instead of
 *     QTreeWidget and manual syncing between plugin list and view
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
#include <QShowEvent>

#include "src/config/Settings.h"
#include "src/db/DatabaseInfo.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/plugin/info/InfoPluginSelectionDialog.h"
#include "src/plugin/settings/PluginSettingsDialog.h"
#include "src/gui/views/ReadOnlyItemDelegate.h"
#include "src/gui/views/SpinBoxCreator.h"
#include "src/gui/views/SpecialIntDelegate.h"
#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/gui/dialogs.h"
#include "src/i18n/notr.h"

#include "src/plugins/weather/ExternalWeatherPlugin.h"

//const int columnTitle=0;
//const int columnCommand=1;
//const int columnEnabled=2;
//const int columnRichText=3;
//const int columnInterval=4;
//const int columnWarnOnDeath=5;

const int captionColumn=0;
const int    nameColumn=1;
const int enabledColumn=2;
const int  configColumn=3;

SettingsWindow::SettingsWindow (QWidget *parent):
	SkDialog<Ui::SettingsWindowClass> (parent),
	warned (false),
	databaseSettingsChanged (false)
{
	// TODO there should be a warning if the settings can't be saved without a password

	ui.setupUi (this);

	ui.dbTypePane->setVisible (false);

	// Make boolean columns and some other columns read-only
	// The title column is read-only because we would have to write back the
	// value to the plugin after editing it so the plugin settings dialog show
	// it correctly.
	ui.infoPluginList->setItemDelegateForColumn (   nameColumn, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn (captionColumn, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn (enabledColumn, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn ( configColumn, new ReadOnlyItemDelegate (ui.infoPluginList));

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
	ui.mysqlDefaultPortCheckBox->setChecked (info.defaultPort);
	ui.mysqlPortInput          ->setValue   (info.port);
	ui.mysqlUserInput          ->setText    (info.username);
	ui.mysqlPasswordInput      ->setText    (info.password);
	ui.mysqlDatabaseInput      ->setText    (info.database);

	// *** Settings
	// Data
	ui.locationInput         ->setText    (s.location);
	ui.recordTowpilotCheckbox->setChecked (s.recordTowpilot);
	ui.checkMedicalsCheckbox ->setChecked (s.checkMedicals);
	// Permissions
	ui.protectSettingsCheckbox      ->setChecked (s.protectSettings);
	ui.protectLaunchMethodsCheckbox ->setChecked (s.protectLaunchMethods);
	ui.protectMergePeopleCheckbox   ->setChecked (s.protectMergePeople);
	ui.protectFlightDatabaseCheckbox->setChecked (s.protectFlightDatabase);
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
	// Plugin selection lists
	ui.weatherPluginInput      ->addItem (notr ("-"), QString ());
	ui.weatherWindowPluginInput->addItem (notr ("-"), QString ());

	QList<const WeatherPlugin::Descriptor *> sortedPlugins (PluginFactory::getInstance ().getDescriptors<WeatherPlugin> ());
	qSort (sortedPlugins.begin (), sortedPlugins.end (), WeatherPlugin::Descriptor::nameLessThanP);
	foreach (const WeatherPlugin::Descriptor *descriptor, sortedPlugins)
	{
		QString name=descriptor->getName ();
		QString id  =descriptor->getId   ();
		ui.weatherPluginInput      ->addItem (name, id);
		ui.weatherWindowPluginInput->addItem (name, id);
	}

	// Weather plugin
	ui.weatherPluginBox          ->setChecked               (s.weatherPluginEnabled);
	ui.weatherPluginInput        ->setCurrentItemByItemData (s.weatherPluginId, 0);
	ui.weatherPluginCommandInput ->setText                  (s.weatherPluginCommand );
	ui.weatherPluginHeightInput  ->setValue                 (s.weatherPluginHeight  );
	ui.weatherPluginIntervalInput->setValue                 (s.weatherPluginInterval/60);
	on_weatherPluginInput_currentIndexChanged ();

	// Weather dialog
	ui.weatherWindowBox          ->setChecked               (s.weatherWindowEnabled);
	ui.weatherWindowPluginInput  ->setCurrentItemByItemData (s.weatherWindowPluginId, 0);
	ui.weatherWindowCommandInput ->setText                  (s.weatherWindowCommand );
	ui.weatherWindowIntervalInput->setValue                 (s.weatherWindowInterval/60);
	ui.weatherWindowTitleInput   ->setText                  (s.weatherWindowTitle   );
	on_weatherWindowPluginInput_currentIndexChanged ();

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
	item->setData       (captionColumn, Qt::DisplayRole, plugin->getCaption ());
	item->setData       (   nameColumn, Qt::DisplayRole, plugin->getName ());
	item->setCheckState (enabledColumn, plugin->isEnabled ()?Qt::Checked:Qt::Unchecked);
	item->setData       ( configColumn, Qt::DisplayRole, plugin->configText ());

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
	s.checkMedicals =ui.checkMedicalsCheckbox ->isChecked ();
	// Permissions
	s.protectSettings      =ui.protectSettingsCheckbox      ->isChecked ();
	s.protectLaunchMethods =ui.protectLaunchMethodsCheckbox ->isChecked ();
	s.protectMergePeople   =ui.protectMergePeopleCheckbox   ->isChecked ();
	s.protectFlightDatabase=ui.protectFlightDatabaseCheckbox->isChecked ();
	// Diagnostics
	s.enableDebug=ui.enableDebugCheckbox->isChecked ();
	s.diagCommand=ui.diagCommandInput   ->text ();

	// *** Plugins - Info
	int numInfoPlugins=infoPlugins.size ();
	assert (numInfoPlugins==ui.infoPluginList->topLevelItemCount ());
	for (int i=0; i<numInfoPlugins; ++i)
	{
		QTreeWidgetItem &item=*ui.infoPluginList->topLevelItem (i);
		infoPlugins[i]->setCaption (item.data       (captionColumn, Qt::DisplayRole).toString ());
		infoPlugins[i]->setEnabled (item.checkState (enabledColumn                 )==Qt::Checked);
	}
	s.writeInfoPlugins (infoPlugins);


	// *** Plugins - Weather
	// Weather plugin
	s.weatherPluginEnabled =ui.weatherPluginBox          ->isChecked ();
	s.weatherPluginId      =ui.weatherPluginInput        ->currentItemData ().toString ();
	s.weatherPluginCommand =ui.weatherPluginCommandInput ->text ();
	s.weatherPluginHeight  =ui.weatherPluginHeightInput  ->value ();
	s.weatherPluginInterval=ui.weatherPluginIntervalInput->value ()*60;
	// Weather dialog
	s.weatherWindowEnabled =ui.weatherWindowBox          ->isChecked ();
	s.weatherWindowPluginId=ui.weatherWindowPluginInput  ->currentItemData ().toString ();
	s.weatherWindowCommand =ui.weatherWindowCommandInput ->text ();
	s.weatherWindowInterval=ui.weatherWindowIntervalInput->value ()*60;
	s.weatherWindowTitle   =ui.weatherWindowTitleInput   ->text ();

	// *** Plugins - Paths
	s.pluginPaths=getPluginPaths ();

	s.save ();

	databaseSettingsChanged=oldInfo.different (info);
}

QStringList SettingsWindow::getPluginPaths ()
{
	QStringList pluginPaths;

	int numPluginPaths=ui.pluginPathList->count ();

	for (int i=0; i<numPluginPaths; ++i)
		pluginPaths << ui.pluginPathList->item (i)->text ();

	return pluginPaths;
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

	QList<const InfoPlugin::Descriptor *> descriptors=PluginFactory::getInstance ().getDescriptors<InfoPlugin> ();
	const InfoPlugin::Descriptor *descriptor=InfoPluginSelectionDialog::select (descriptors, this);

	if (!descriptor) return;

	InfoPlugin *plugin=descriptor->create ();

	if (plugin)
	{
		plugin->setCaption (plugin->getName ()+notr (":"));
		int settingsDialogResult=PluginSettingsDialog::invoke (plugin, this, this);

		if (settingsDialogResult==QDialog::Accepted)
		{
			infoPlugins.append (plugin);

			QTreeWidgetItem *item=new QTreeWidgetItem (list);
			readItem (item, plugin);

			list->setCurrentItem (item);
		}
		else
		{
			delete plugin;
		}
	}
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

void SettingsWindow::on_infoPluginSettingsButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;

	PluginSettingsDialog::invoke (infoPlugins[row], this, this);
	readItem (ui.infoPluginList->topLevelItem (row), infoPlugins[row]);
}

void SettingsWindow::on_infoPluginList_itemDoubleClicked (QTreeWidgetItem *item, int column)
{
	(void)column;
	if (!item) return;

	ui.infoPluginList->setCurrentItem (item);
	on_infoPluginSettingsButton_clicked ();
}

/**
 * Determines whether the settings may be stored.
 *
 * Changing the settings may be protected, that is, require that the database
 * password is entered. However, it's not simply a matter of asking the password
 * in this case. This method determines whether the settings may be store, and
 * asks the user for a password if required.
 */
bool SettingsWindow::allowEdit ()
{
	QString message;
	QString requiredPassword;

	QString oldPassword=Settings::instance ().databaseInfo.password;
	QString newPassword=ui.mysqlPasswordInput->text ();
	bool passwordChanged=(newPassword!=oldPassword);

	if (Settings::instance ().protectSettings)
	{
		// The password protection is (already was) enabled. The user must enter
		// the password.
		// If the password was also changed, clarify that the user must enter
		// the *old* password.
		if (passwordChanged)
			message=tr ("The (old) database password must be entered to save\nthe settings.");
		else
			message=tr ("The database password must be entered to save\nthe settings.");
		requiredPassword=oldPassword;
	}
	else if (ui.protectSettingsCheckbox->isChecked ())
	{
		// The password protection was disabled before, but has been enabled.
		// The user must enter the database password to make sure that he
		// actually knows the password, so he won't enable the protection
		// without having a way to disable it again.
		// If the password was also changed, clarify that the user must enter
		// the *new* password.
		if (passwordChanged)
			message=tr (
				"Password protection of the settings is being enabled. The\n"
				"(new) database password must be entered. If you don't want\n"
				"to enable the protection, you can cancel now and disable\n"
				"the corresponding option."
				);
		else
			message=tr (
				"Password protection of the settings is being enabled. The\n"
				"database password must be entered. If you don't want\n"
				"to enable the protection, you can cancel now and disable\n"
				"the corresponding option."
				);
		requiredPassword=newPassword;
	}
	else
	{
		// The password protection is not currently enabled, nor has it been
		// enabled. We can store the settings.
		return true;
	}

	// If we didn't return yet, we may only store the settings if the user
	// enters the correct password. Which one is the "correct" password has been
	// determined before.
	return verifyPassword (this, requiredPassword, message);
}

void SettingsWindow::warnEdit ()
{
	if (!Settings::instance ().protectSettings) return;
	if (warned) return;

	showWarning (tr ("Settings protected"), tr (
		"The settings are protected. The settings\n"
		"can be changed, but to save them, the database\n"
		"password must be entered."), this);

	warned=true;
}

void SettingsWindow::on_weatherPluginInput_currentIndexChanged ()
{
	bool external=(ui.weatherPluginInput->currentItemData ().toString ()==ExternalWeatherPlugin::_getId ());
	ui.weatherPluginCommandLabel->setEnabled (external);
	ui.weatherPluginCommandInput->setEnabled (external);
	ui.browseWeatherPluginCommandButton->setEnabled (external);
}

void SettingsWindow::on_weatherWindowPluginInput_currentIndexChanged ()
{
	bool external=(ui.weatherWindowPluginInput->currentItemData ().toString ()==ExternalWeatherPlugin::_getId ());
	ui.weatherWindowCommandLabel->setEnabled (external);
	ui.weatherWindowCommandInput->setEnabled (external);
	ui.browseWeatherWindowCommandButton->setEnabled (external);
}

void SettingsWindow::on_browseWeatherPluginCommandButton_clicked ()
{
	QString filename=Plugin::browse (ui.weatherPluginCommandInput->text (), notr ("*"), getPluginPaths (), this);

	if (!filename.isEmpty ())
		ui.weatherPluginCommandInput->setText (filename);
}

void SettingsWindow::on_browseWeatherWindowCommandButton_clicked ()
{
	QString filename=Plugin::browse (ui.weatherWindowCommandInput->text (), notr ("*"), getPluginPaths (), this);

	if (!filename.isEmpty ())
		ui.weatherWindowCommandInput->setText (filename);
}

void SettingsWindow::showEvent (QShowEvent *event)
{
	// When the dialog is made visible (showEvent), show a warning if the
	// settings are password protected.  This warning will only be shown once
	// during the lifetime of the SettingsWindow instance.
	// Note that when this is active, all other calls to warnEdit are
	// unnecessary (but harmless).
	if (!event->spontaneous ())
		warnEdit ();
}
