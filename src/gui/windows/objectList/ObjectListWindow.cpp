#include "ObjectListWindow.h"

/*
 * Improvements:
 *   - restore cursor position after refresh
 *   - add a way to refresh the list from the database
 *   - on deletion: add identification (plane: registration; person: formatted
 *     name)
 *   - display centered
 */

/*
 * FIXME plan for merging people:
 *   (0) preparation
 *   (A) allow type specific actions in ObjectListWindow
 *   (B) implement merging
 *   (X) also
 *
 * (0)
 *   - test ObjectSelectWindow for a large database (test code below)
 *
 * (A): currently, we only have generic code in ObjectListWindow<T>. To allow
 * type specific actions:
 *   - put specific code in PersonListWindow
 *
 * (B)
 *   - use a selector for selecting a person
 *   - make a confirmation dialog
 *
	#include "src/gui/windows/ObjectSelectWindow.h"
	EntityList<Person> testList=manager.getCache ().getObjects<Person> ();
	dbId resultId;
	ObjectSelectWindow<Person>::select (&resultId, "Test", "Test auswählen", testList.getList (), invalidId, parent);
 *
 */

#include <iostream>

#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QKeyEvent>

#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/gui/dialogs.h"
#include "src/text.h"
#include "src/model/objectList/AutomaticEntityList.h"
#include "src/model/objectList/ObjectModel.h"


//template<class T> ObjectListWindow<T>::ObjectListWindow (DbManager &manager, ObjectListModel<T> *listModel, bool listModelOwned, QWidget *parent):
//	ObjectListWindowBase (manager, parent),
//	listModel (listModel), listModelOwned (listModelOwned)
// FIXME ist there a way to prevent calling this, even privately, except
// through the create method?
template<class T> ObjectListWindow<T>::ObjectListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindowBase (manager, parent),
	contextMenu (new QMenu (this))
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

	// TODO this should be done later - a subclass may add menus
	QString mnemonics="f";
	QChar closeButtonMnemonic=getMnemonic (ui.buttonBox->button (QDialogButtonBox::Close));
	if (!closeButtonMnemonic.isNull ()) mnemonics+=closeButtonMnemonic;
	ui.menuObject->setTitle (insertMnemonic (T::objectTypeDescription (), mnemonics));
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
	ObjectListWindowBase *window = ObjectListWindow<T>::create (manager, parent);
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
	if (!listIndex.isValid ()) return;

	ObjectEditorWindow<T>::editObject (this, manager, listModel->at (listIndex));
}

template<class T> void ObjectListWindow<T>::on_actionDelete_triggered ()
{
	if (!allowEdit (makePasswordMessage ())) return;

	QModelIndex listIndex=proxyModel->mapToSource (ui.table->currentIndex ());
	if (!listIndex.isValid ()) return;
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

/**
 * Not using the activated signal because it may be emitted on single click,
 * depending on the desktop settings.
 */
template<class T> void ObjectListWindow<T>::on_table_doubleClicked (const QModelIndex &index)
{
	if (!allowEdit (makePasswordMessage ())) return;

	if (index.isValid ())
	{
		QModelIndex listIndex=proxyModel->mapToSource (index);
		if (!listIndex.isValid ()) return;
		ObjectEditorWindow<T>::editObject (this, manager, listModel->at (listIndex));
	}
	else
	{
		ui.actionNew->trigger ();
	}
}

template<class T> void ObjectListWindow<T>::prepareContextMenu (QMenu *contextMenu, const QPoint &pos)
{
	// Note that contextMenu refers to the parameter, not the private class
	// property
	contextMenu->clear ();

	if (ui.table->indexAt (pos).isValid ())
	{
		contextMenu->addAction (ui.actionNew);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionEdit);
		contextMenu->addAction (ui.actionDelete);
	}
	else
	{
		contextMenu->addAction (ui.actionNew);
	}
}

template<class T> void ObjectListWindow<T>::on_table_customContextMenuRequested (const QPoint &pos)
{
	prepareContextMenu (contextMenu, pos);
	contextMenu->popup (ui.table->mapToGlobal (pos), 0);
}

template<class T> void ObjectListWindow<T>::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "ObjectListWindow key " << e->key () << std::endl;

	int key=e->key ();

	// Treat return/enter like double click on the current item
	if (key==Qt::Key_Enter || key==Qt::Key_Return)
	{
		if (ui.table->hasFocus ())
		{
			QModelIndex index=ui.table->currentIndex ();
			if (index.isValid ())
			{
				on_table_doubleClicked (index);
				return;
			}
		}
	}

	ObjectListWindowBase::keyPressEvent (e);
}


template<class T> QString ObjectListWindow<T>::makePasswordMessage ()
{
	return utf8 ("Zum Editieren der %1 ist das Datenbankpasswort erforderlich.").arg (T::objectTypeDescriptionPlural ());
}

/**
 * FIXME document:
 *   - must be used for all ObjectListWindow creations
 *   - defaults to creating ObjectListWindow<T>
 *   - can be specialized (event though instantiated below) to create a subclass
 */
template<class T> ObjectListWindow<T> *ObjectListWindow<T>::create (DbManager &manager, QWidget *parent)
{
	return new ObjectListWindow<T> (manager, parent);
}

// Instantiate the class templates
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"

template class ObjectListWindow<Plane>;
template class ObjectListWindow<Person>;
template class ObjectListWindow<LaunchMethod>;
