#include "LaunchMethodSelectionWindow.h"

#include <QPushButton>

#include "src/db/cache/Cache.h"
#include "src/gui/dialogs.h"
#include "src/util/qString.h"

LaunchMethodSelectionWindow::LaunchMethodSelectionWindow (QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);
	ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");
}

LaunchMethodSelectionWindow::~LaunchMethodSelectionWindow ()
{

}

bool LaunchMethodSelectionWindow::select (Cache &cache, dbId &value, QWidget *parent)
{
	if (cache.getLaunchMethods ().getList ().isEmpty ())
	{
		showWarning (utf8 ("Keine Startarten definiert"), utf8 ("Es kann keine Startart vorausgewählt werden, da keine Startarten definiert sind."), parent);
		return false;
	}

	LaunchMethodSelectionWindow *window=new LaunchMethodSelectionWindow (parent);
	window->setModal (true);

	foreach (const LaunchMethod &launchMethod, cache.getLaunchMethods ().getList ())
		window->ui.launchMethodInput->addItem (launchMethod.nameWithShortcut (), launchMethod.getId ());

	window->ui.preselectionCheckbox->setChecked (idValid (value));
	if (idValid (value))
		window->ui.launchMethodInput->setCurrentItemByItemData (value);

	if (window->exec ()!=QDialog::Accepted) return false;

	if (window->ui.preselectionCheckbox->isChecked ())
		value=window->ui.launchMethodInput->currentItemData ().toLongLong ();
	else
		value=invalidId;

	return true;
}
