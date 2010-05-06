#include "ObjectListWindow.h"

/*
 * Improvements:
 *   - restore cursor position after refresh
 *   - add a way to refresh the list from the database
 *   - on deletion: add identification (plane: registration; person: formatted
 *     name)
 *   - display centered
 */
#include <iostream>

#include <QMessageBox>
#include <QSortFilterProxyModel>

#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/gui/dialogs.h"
#include "src/text.h"
#include "src/model/objectList/AutomaticEntityList.h"
#include "src/model/objectList/ObjectModel.h"


//template<class T> ObjectListWindow<T>::ObjectListWindow (DbManager &manager, ObjectListModel<T> *listModel, bool listModelOwned, QWidget *parent):
//	ObjectListWindowBase (manager, parent),
//	listModel (listModel), listModelOwned (listModelOwned)
template<class T> ObjectListWindow<T>::ObjectListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindowBase (manager, parent)
{
	// Create the object listModel
	list = new AutomaticEntityList<T>
		(manager.getCache (), manager.getCache ().getObjects<T> ().getList (), this);

	// Create the object model and object list model
	objectModel=new typename T::DefaultObjectModel ();
	listModel = new ObjectListModel<T> (list, false, objectModel, true, this);


	// Set the list model as the table's model with a sort proxy
	proxyModel=new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (listModel);
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
	// list, listModel and proxyModel are deleted automatically by QWidget
	// objectModel ist deleted automatically be objectListModel
}

template<class T> void ObjectListWindow<T>::show (DbManager &manager, QWidget *parent)
{
	show (manager, false, "", parent);
}

template<class T> void ObjectListWindow<T>::show (DbManager &manager, const QString &password, QWidget *parent)
{
	show (manager, true, password, parent);
}

template<class T> void ObjectListWindow<T>::show (DbManager &manager, bool editPasswordRequired, const QString &editPassword, QWidget *parent)
{
	// Create the window
	ObjectListWindowBase *window = new ObjectListWindow<T> (manager, parent);
	window->setAttribute (Qt::WA_DeleteOnClose, true);

	if (editPasswordRequired)
		window->requireEditPassword (editPassword);

	// Show the window
	window->show ();
}


template<class T> void ObjectListWindow<T>::on_actionNew_triggered ()
{
	if (!allowEdit (makePasswordMessage ())) return;

	ObjectEditorWindow<T>::createObject (this, manager);
}

template<class T> void ObjectListWindow<T>::on_actionEdit_triggered ()
{
	if (!allowEdit (makePasswordMessage ())) return;

	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	ObjectEditorWindow<T>::editObject (this, manager, listModel->at (listIndex));
}

template<class T> void ObjectListWindow<T>::on_actionDelete_triggered ()
{
	if (!allowEdit (makePasswordMessage ())) return;

	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	const T &object=listModel->at (listIndex);
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
		QString text=utf8 ("%1 %2 wird verwendet und kann daher nicht gelöscht werden.").arg (T::objectTypeDescriptionDefinite (), object.getDisplayName ());
		QMessageBox::critical (this, title, firstToUpper (text));
	}
	else
	{
		QString title=utf8 ("%1 löschen?").arg (T::objectTypeDescription ());
		QString question=utf8 ("Soll %1 %2 gelöscht werden?").arg (T::objectTypeDescriptionDefinite (), object.getDisplayName ());
		if (yesNoQuestion (this, title, question))
		{
			try
			{
				QModelIndex previousIndex=ui.table->currentIndex ();
				manager.deleteObject<T> (id, this);
				ui.table->setCurrentIndex (previousIndex); // Handles deletion of last item correctly
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
	try
	{
//		manager.refreshCache (this);
		manager.refreshObjects<T> (this);
	}
	catch (OperationCanceledException &ex) {}

	list->replaceList (manager.getCache ().getObjects<T> ().getList ());
}

template<class T> void ObjectListWindow<T>::on_table_activated (const QModelIndex &index)
{
	if (!allowEdit (makePasswordMessage ())) return;

	QModelIndex listIndex=proxyModel->mapToSource (index);
	ObjectEditorWindow<T>::editObject (this, manager, listModel->at (listIndex));
}

template<class T> QString ObjectListWindow<T>::makePasswordMessage ()
{
	return utf8 ("Zum Editieren der %1 ist das Datenbankpasswort erforderlich.").arg (T::objectTypeDescriptionPlural ());
}

// Instantiate the class templates
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"

template class ObjectListWindow<Plane>;
template class ObjectListWindow<Person>;
template class ObjectListWindow<LaunchMethod>;
