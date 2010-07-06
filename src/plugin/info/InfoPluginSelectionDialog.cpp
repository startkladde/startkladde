#include "InfoPluginSelectionDialog.h"

#include <QPushButton>

const int nameColumn=0;
const int descriptionColumn=1;

InfoPluginSelectionDialog::InfoPluginSelectionDialog (const QList<InfoPlugin::Descriptor *> &plugins, QWidget *parent):
    QDialog (parent),
    plugins (plugins)
{
	ui.setupUi (this);
	ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");

	setup ();
}

InfoPluginSelectionDialog::~InfoPluginSelectionDialog()
{

}

void InfoPluginSelectionDialog::setup ()
{
	ui.pluginList->clear ();

	if (plugins.isEmpty ())
	{
		ui.pluginList->setVisible (false);
		ui.messageLabel->setText ("Es sind keine Plugins vorhanden.");
		ui.buttonBox->button (QDialogButtonBox::Ok)->setEnabled (false);
	}
	else
	{
		foreach (InfoPlugin::Descriptor *descriptor, plugins)
		{
			QTreeWidgetItem *item=new QTreeWidgetItem (ui.pluginList);
			item->setData (nameColumn       , Qt::DisplayRole, descriptor->getName        ());
			item->setData (descriptionColumn, Qt::DisplayRole, descriptor->getDescription ());
		}

		for (int i=0; i<ui.pluginList->columnCount (); ++i)
			ui.pluginList->resizeColumnToContents (i);
	}
}

const InfoPlugin::Descriptor *InfoPluginSelectionDialog::getCurrentPluginDescriptor ()
{
 	QTreeWidget *list=ui.pluginList;
	int row=ui.pluginList->indexOfTopLevelItem (list->currentItem ());

	if (row<0 || row>=list->topLevelItemCount ())
		return NULL;
	else
		return plugins[row];
}

const InfoPlugin::Descriptor *InfoPluginSelectionDialog::select (const QList<InfoPlugin::Descriptor *> &plugins, QWidget *parent)
{
	InfoPluginSelectionDialog *dialog=new InfoPluginSelectionDialog (plugins, parent);
	dialog->setModal (true);

	const InfoPlugin::Descriptor *result=NULL;
	if (dialog->exec ()==QDialog::Accepted)
		result=dialog->getCurrentPluginDescriptor ();

	delete dialog;

	return result;
}
