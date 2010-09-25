/*
 * PersonListWindow.cpp
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#include "PersonListWindow.h"

PersonListWindow::PersonListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindow<Person> (manager, parent)
{
}

PersonListWindow::~PersonListWindow ()
{
}

template<> ObjectListWindow<Person> *ObjectListWindow<Person>::create (DbManager &manager, QWidget *parent)
{
	return new ObjectListWindow<Person> (manager, parent);
}

