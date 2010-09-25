/*
 * PersonListWindow.h
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#ifndef PERSONLISTWINDOW_H_
#define PERSONLISTWINDOW_H_

#include "src/gui/windows/objectList/ObjectListWindow.h"

#include "src/model/Person.h"

class DbManager;

class PersonListWindow: public ObjectListWindow<Person>
{
	public:
		PersonListWindow (DbManager &manager, QWidget *parent=NULL);
		virtual ~PersonListWindow ();
};

#endif /* PERSONLISTWINDOW_H_ */
