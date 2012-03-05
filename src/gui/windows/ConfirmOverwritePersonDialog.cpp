#include "ConfirmOverwritePersonDialog.h"

#include <QPushButton>

#include "src/util/qString.h"
#include "src/model/Person.h"
#include "src/model/objectList/ObjectModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/model/objectList/MutableObjectList.h"

/**
 * Creates a ConfirmOverwritePersonDialog
 *
 * @param parent the Qt parent widgets
 * @param f the Qt window flags
 */
ConfirmOverwritePersonDialog::ConfirmOverwritePersonDialog (QWidget *parent, Qt::WindowFlags f):
	QDialog (parent, f)
{
	ui.setupUi(this);
	//ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");
}

ConfirmOverwritePersonDialog::~ConfirmOverwritePersonDialog ()
{
}

/**
 * Sets up the window
 *
 * This method clears, sets up and populates the tree view with the correct
 * and wrong people, and adjusts some labels.
 *
 * @param correctPerson the correct person entry
 * @param wrongPeople a list of one or more wrong person entries
 */
void ConfirmOverwritePersonDialog::setup (const Person &correctPerson, const QList<Person> &wrongPeople)
{
	// The model to get the data from
	Person::DefaultObjectModel model;

	QString wrongEntriesText;
	if (wrongPeople.size ()>1)
	{
		wrongEntriesText=tr ("Erroneous entries");
		ui.introLabel->setText (tr ("The following erroneous entries will be replaced with the correct entry:"));
		// TODO set complete text
		ui.descriptionLabel->setText (ui.descriptionLabel->text ().arg (tr ("All"), tr ("people")));
	}
	else
	{
		wrongEntriesText=tr ("Wrong entry");
		ui.introLabel->setText (tr ("The following erroneous entry will be replaced with the correct entry:"));
		// TODO set complete text
		ui.descriptionLabel->setText (ui.descriptionLabel->text ().arg (tr ("Both"), tr ("person")));
	}

	// Clear the tree
	ui.entriesTree->clear ();

	// Set up the header
	ui.entriesTree->setHeaderLabels (model.displayHeaderStrings ());

	// Create the root items
	QTreeWidgetItem *  wrongParentItem=new QTreeWidgetItem (ui.entriesTree, QStringList (wrongEntriesText          ));
	QTreeWidgetItem *correctParentItem=new QTreeWidgetItem (ui.entriesTree, QStringList (tr ("Correct entry")));

	// Set up the root items
	wrongParentItem  ->setFirstColumnSpanned (true);
	correctParentItem->setFirstColumnSpanned (true);

	// Create a child item for the corrent person
	new QTreeWidgetItem (correctParentItem, model.displayDataStrings (correctPerson));

	// Create child items for the wrong people
	foreach (const Person &person, wrongPeople)
		new QTreeWidgetItem (wrongParentItem, model.displayDataStrings (person));

	// Expand the root items
	wrongParentItem  ->setExpanded (true);
	correctParentItem->setExpanded (true);

	// Resize the columns to the contents
	for (int i=0; i<model.columnCount (); ++i)
		ui.entriesTree->resizeColumnToContents (i);
}

/**
 * Invokes the ConfirmOverwritePersonDialog
 *
 * @param correctPerson the correct person entry
 * @param wrongPeople a list of one or more wrong person entries
 * @param parent the Qt parent widget
 * @return true if the user accepted the operation, false otherwise
 */
bool ConfirmOverwritePersonDialog::confirmOverwrite (const Person &correctPerson, const QList<Person> &wrongPeople, QWidget *parent)
{
	ConfirmOverwritePersonDialog dialog (parent);
	dialog.setModal (true);

	dialog.setup (correctPerson, wrongPeople);

	if (QDialog::Accepted==dialog.exec ())
		return true;
	else
		return false;
}
