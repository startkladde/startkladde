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
	mergeAction (new QAction (utf8 ("&Zusammenfassen"), this)),
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
	if (!mergePermission.permit ("Zum Zusammenfassen von Personen ist das Datenbankpasswort erforderlich."))
		return;

	QList<Person> people=activeObjects ();
	if (people.size ()<2)
	{
		showWarning (
			utf8 ("Zu wenige Person ausgewählt"),
			utf8 ("Zum Zusammenfassen müssen mindestens zwei Personen ausgewählt sein."),
			this);
		return;
	}

	QString title=utf8 ("Korrekten Eintrag auswählen");
	QString text=utf8 ("Bitte den korrekten Eintrag auswählen.");

	if (people.size ()>2)
		text+=utf8 (" Alle anderen Einträge werden überschrieben.");
	else
		text+=utf8 (" Der andere Eintrag wird überschrieben.");

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
