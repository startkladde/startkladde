#include "ObjectListWindow.h"

/*
 * Improvements:
 *   - restore cursor position after refresh
 *   - add a way to refresh the list from the database
 *   - on deletion: add identification (plane: registration; person: formatted
 *     name)
 */
#include <iostream>

#include <QSortFilterProxyModel>

#include "src/model/objectList/AbstractObjectList.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/db/task/ObjectUsedTask.h"
#include "src/db/task/DeleteObjectTask.h"

template<class T> ObjectListWindow<T>::ObjectListWindow (DataStorage &dataStorage, ObjectListModel<T> *list, bool listOwned, QWidget *parent):
	ObjectListWindowBase (parent),
    dataStorage (dataStorage), list (list), listOwned (listOwned)
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
	ObjectEditorWindow<T>::createObject (this, dataStorage);
}

template<class T> void ObjectListWindow<T>::on_actionEdit_triggered ()
{
	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	ObjectEditorWindow<T>::editObject (this, dataStorage, list->at (listIndex));
}

template<class T> void ObjectListWindow<T>::on_actionDelete_triggered ()
{
	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	const T &object=list->at (listIndex);
	db_id id=object.get_id ();

	ObjectUsedTask<T> checkTask (dataStorage, id);
	dataStorage.addTask (&checkTask);
	TaskProgressDialog::waitTask (this, &checkTask);
	if (checkTask.isCanceled ()) return;
	if (!checkTask.isCompleted ()) return; // TODO error check instead
	bool objectUsed=checkTask.getResult ();

	if (objectUsed)
	{
		QString title=QString::fromUtf8 ("%1 benutzt").arg (T::objectTypeDescription ());
		QString text=QString::fromUtf8 ("%1 wird verwendet und kann daher nicht gelöscht werden.").arg (T::objectTypeDescriptionDefinite ());
		QMessageBox::critical (this, title, firstToUpper (text));
	}
	else
	{
		QString title=QString::fromUtf8 ("%1 löschen?").arg (T::objectTypeDescription ());
		QString question=QString::fromUtf8 ("Soll %1 gelöscht werden?").arg (T::objectTypeDescriptionDefinite ());
		if (yesNoQuestion (this, title, question))
		{
			DeleteObjectTask<T> deleteTask (dataStorage, id);
			dataStorage.addTask (&deleteTask);
			TaskProgressDialog::waitTask (this, &deleteTask);
		}
	}
}

template<class T> void ObjectListWindow<T>::on_actionRefresh_triggered ()
{
	dataStorage.refreshViews<T> ();
}

template<class T> void ObjectListWindow<T>::on_table_activated (const QModelIndex &index)
{
	QModelIndex listIndex=proxyModel->mapToSource (index);
	ObjectEditorWindow<T>::editObject (this, dataStorage, list->at (listIndex));
}


// Instantiate the class templates
#include "src/model/Plane.h"
#include "src/model/Person.h"

template class ObjectListWindow<Plane>;
template class ObjectListWindow<Person>;
