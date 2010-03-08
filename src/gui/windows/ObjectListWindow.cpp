#include "ObjectListWindow.h"

/*
 * Improvements:
 *   - restore cursor position after refresh
 *   - add a way to refresh the list from the database
 *   - on deletion: add identification (plane: registration; person: formatted
 *     name)
 */
#include <iostream>

#include <QMessageBox>
#include <QSortFilterProxyModel>

#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/gui/dialogs.h"
#include "src/text.h"
#include "src/db/DbManager.h"

template<class T> ObjectListWindow<T>::ObjectListWindow (DbManager &manager, ObjectListModel<T> *list, bool listOwned, QWidget *parent):
	ObjectListWindowBase (parent),
	manager (manager), list (list), listOwned (listOwned)
{
	// Set the list as the table's model with a sort proxy
	proxyModel=new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (list);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);
	ui.table->setModel (proxyModel);

	ui.table->setAutoResizeRows (true);
	ui.table->sortByColumn (0, Qt::AscendingOrder);
	ui.table->resizeColumnsToContents ();
	ui.table->resizeRowsToContents (); // Do this although autoResizeRows is true - there are already rows.

	setWindowTitle (T::objectTypeDescriptionPlural ());
}

template<class T> ObjectListWindow<T>::~ObjectListWindow()
{
	if (listOwned)
		delete list;
}


template<class T> void ObjectListWindow<T>::on_actionNew_triggered ()
{
	ObjectEditorWindow<T>::createObject (this, manager);
}

template<class T> void ObjectListWindow<T>::on_actionEdit_triggered ()
{
	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	ObjectEditorWindow<T>::editObject (this, manager, list->at (listIndex));
}

template<class T> void ObjectListWindow<T>::on_actionDelete_triggered ()
{
	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	const T &object=list->at (listIndex);
	dbId id=object.getId ();

	bool objectUsed=true;

	try
	{
		objectUsed=manager.objectUsed<T> (id, this);
	}
	catch (OperationCanceledException)
	{
		return;
	}

	if (objectUsed)
	{
		QString title=utf8 ("%1 benutzt").arg (T::objectTypeDescription ());
		QString text=utf8 ("%1 wird verwendet und kann daher nicht gelöscht werden.").arg (T::objectTypeDescriptionDefinite ());
		QMessageBox::critical (this, title, firstToUpper (text));
	}
	else
	{
		// TODO include name in question
		QString title=utf8 ("%1 löschen?").arg (T::objectTypeDescription ());
		QString question=utf8 ("Soll %1 gelöscht werden?").arg (T::objectTypeDescriptionDefinite ());
		if (yesNoQuestion (this, title, question))
		{
			try
			{
				manager.deleteObject<T> (id, this);
			}
			catch (OperationCanceledException)
			{
				// TODO the cache may now be inconsistent
			}

			// TODO select "next" entry
		}
	}
}

template<class T> void ObjectListWindow<T>::on_actionRefresh_triggered ()
{
	// TODO
}

template<class T> void ObjectListWindow<T>::on_table_activated (const QModelIndex &index)
{
	QModelIndex listIndex=proxyModel->mapToSource (index);
	ObjectEditorWindow<T>::editObject (this, manager, list->at (listIndex));
}


// Instantiate the class templates
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"

template class ObjectListWindow<Plane>;
template class ObjectListWindow<Person>;
template class ObjectListWindow<LaunchMethod>;
