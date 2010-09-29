/*
 * PersonListWindow.cpp
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#include "PersonListWindow.h"

#include <iostream>

#include "src/util/qString.h"

PersonListWindow::PersonListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindow<Person> (manager, parent),
	overwriteAction (new QAction (utf8 ("Über&schreiben"), this))
{
	connect (overwriteAction, SIGNAL (triggered ()), this, SLOT (overwriteAction_triggered ()));

	ui.menuObject->addSeparator ();
	ui.menuObject->addAction (overwriteAction);
}

PersonListWindow::~PersonListWindow ()
{
}

template<> ObjectListWindow<Person> *ObjectListWindow<Person>::create (DbManager &manager, QWidget *parent)
{
	return new PersonListWindow (manager, parent);
}

void PersonListWindow::overwriteAction_triggered ()
{
	if (!allowEdit (makePasswordMessage ())) return;

	const Person *person=getCurrentObject ();
	if (!person) return;


	// FIXME implement
	std::cout << "overwrite person " << person->getDisplayName () << std::endl;
}

void PersonListWindow::prepareContextMenu (QMenu *contextMenu, const QPoint &pos)
{
	ObjectListWindow<Person>::prepareContextMenu (contextMenu, pos);

	if (ui.table->indexAt (pos).isValid ())
	{
		contextMenu->addSeparator ();
		contextMenu->addAction (overwriteAction);
	}
}

