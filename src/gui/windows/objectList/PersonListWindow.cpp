/*
 * PersonListWindow.cpp
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#include "PersonListWindow.h"

#include <iostream>

#include "src/gui/dialogs.h"
#include "src/util/qString.h"
#include "src/gui/windows/ObjectSelectWindow.h"
#include "src/gui/windows/ConfirmOverwritePersonDialog.h"

PersonListWindow::PersonListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindow<Person> (manager, parent),
	mergeAction (new QAction (utf8 ("&Zusammenfassen"), this))
{
	connect (mergeAction, SIGNAL (triggered ()), this, SLOT (mergeAction_triggered ()));

	ui.menuObject->addSeparator ();
	ui.menuObject->addAction (mergeAction);
}

PersonListWindow::~PersonListWindow ()
{
}

template<> ObjectListWindow<Person> *ObjectListWindow<Person>::create (DbManager &manager, QWidget *parent)
{
	return new PersonListWindow (manager, parent);
}

void PersonListWindow::mergeAction_triggered ()
{
	if (!allowEdit ("Zum Zusammenfassen von Personen ist das Datenbankpasswort erforderlich.")) return;

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
		// FIXME need parent for status indicator?
		manager.mergePeople (correctPerson, people);
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
