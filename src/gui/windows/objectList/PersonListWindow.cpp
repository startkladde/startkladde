/*
 * PersonListWindow.cpp
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#include "PersonListWindow.h"

#include <iostream>

#include "src/config/Settings.h"
#include "src/gui/dialogs.h"
#include "src/util/qString.h"
#include "src/gui/windows/ObjectSelectWindow.h"
#include "src/gui/windows/ConfirmOverwritePersonDialog.h"

PersonListWindow::PersonListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindow<Person> (manager, parent),
	mergeAction (new QAction (tr ("&Merge"), this)),
	mergePermission (this)
{
	connect (mergeAction, SIGNAL (triggered ()), this, SLOT (mergeAction_triggered ()));

	ui.menuObject->addSeparator ();
	ui.menuObject->addAction (mergeAction);

	// The PersonListWindow may be created as ObjectListWindow<Person>. In this
	// case, the specific properties pertaining to merge operations cannot be
	// set because the generic interface does not know about them. Therefore,
	// we explicitly set them here.
	if (Settings::instance ().protectMergePeople)
		mergePermission.requirePassword (Settings::instance ().databaseInfo.password);
}

PersonListWindow::~PersonListWindow ()
{
}

void PersonListWindow::mergeAction_triggered ()
{
	// We cannot use allowEdit because that is also used for adding and editing
	// people, and we may want to reqire a password for merging, but not for
	// editing.
	if (!mergePermission.permit (tr ("The database password must be entered to merge people.")))
		return;

	QList<Person> people=activeObjects ();
	if (people.size ()<2)
	{
		showWarning (
			tr ("Not enough people selected"),
			tr ("At least two people must be selected for merging."),
			this);
		return;
	}

	QString title=tr ("Select correct entry");

	QString text;
	if (people.size ()>2)
		text=tr ("Please select the correct entry. All other entries will be overwritten.");
	else
		text=tr ("Please select the correct entry. The other entry will be overwritten.");

	dbId correctPersonId=invalidId;
	ObjectSelectWindowBase::Result selectionResult=
		ObjectSelectWindow<Person>::select (&correctPersonId, title, text,
			people, new Person::DefaultObjectModel (), true, invalidId, false, this);

	switch (selectionResult)
	{
		case ObjectSelectWindowBase::resultOk:
			break;
		case ObjectSelectWindowBase::resultUnknown:
			// fallthrough
		case ObjectSelectWindowBase::resultNew:
			// fallthrough
		case ObjectSelectWindowBase::resultCancelled: case ObjectSelectWindowBase::resultNoneSelected:
			return;
	}

	Person correctPerson;

	for (int i=0; i<people.size (); ++i)
	{
		if (people.at (i).getId ()==correctPersonId)
		{
			correctPerson=people.takeAt (i);
			break;
		}
	}

	// The wrong people are now in people
	bool confirmed=ConfirmOverwritePersonDialog::confirmOverwrite (correctPerson, people, this);

	if (confirmed)
		// Does not throw OperationCanceledException
		manager.mergePeople (correctPerson, people, this);
}

void PersonListWindow::prepareContextMenu (QMenu *contextMenu)
{
	ObjectListWindow<Person>::prepareContextMenu (contextMenu);

	if (activeObjectCount ()>1)
	{
		contextMenu->addSeparator ();
		contextMenu->addAction (mergeAction);
	}
}
