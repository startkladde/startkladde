/*
 * ObjectSelectWindow.h
 *
 *  Created on: 23.03.2010
 *      Author: deffi
 */

#ifndef OBJECTSELECTWINDOW_H_
#define OBJECTSELECTWINDOW_H_

#include "src/gui/windows/ObjectSelectWindowBase.h"

template<class T> class ObjectModel;

/**
 * A dialog for letting the user select an object from a list. A "new" and
 * "unknown" entry are also given.
 *
 * This template must be instantiated for every class it is used with (at the
 * end of ObjectSelectWindow.cpp).
 */
template<class T> class ObjectSelectWindow: public ObjectSelectWindowBase
{
	public:
		ObjectSelectWindow (const QList<T> &objects, ObjectModel<T> *model, bool modelOwned, dbId selectedId, bool enableSpecialEntries, QWidget *parent=NULL);
		virtual ~ObjectSelectWindow ();

		static Result select (dbId *resultId, const QString &title, const QString &text, const QList<T> &objects, ObjectModel<T> *model, bool modelOwned, dbId preselectionId, bool enableSpecialEntries, QWidget *parent=NULL);

	private:
		ObjectModel<T> *model;
		bool modelOwned;
};

#endif
