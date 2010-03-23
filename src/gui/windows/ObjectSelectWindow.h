/*
 * ObjectSelectWindow.h
 *
 *  Created on: 23.03.2010
 *      Author: deffi
 */

#ifndef OBJECTSELECTWINDOW_H_
#define OBJECTSELECTWINDOW_H_

#include "src/gui/windows/ObjectSelectWindowBase.h"

/**
 * A dialog for letting the user select an object from a list. A "new" and
 * "unknown" entry are also given.
 *
 * Classes to be used with this window must implement get_selector_caption
 * and get_selector_value (see Person). Also, this template must be
 * instantiated for use with the class (at the end of ObjectSelectWindow.cpp).
 */
template<class T> class ObjectSelectWindow: public ObjectSelectWindowBase
{
	public:
		ObjectSelectWindow (const QList<T> &objects, dbId selectedId, QWidget *parent=NULL);
		virtual ~ObjectSelectWindow ();

		static Result select (dbId *resultId, const QString &title, const QString &text, const QList<T> &objects, dbId preselectionId, QWidget *parent=NULL);
};

#endif
