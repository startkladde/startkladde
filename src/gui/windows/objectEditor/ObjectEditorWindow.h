#ifndef OBJECTEDITORWINDOW_H
#define OBJECTEDITORWINDOW_H

#include <cassert>

// TODO reduce dependencies
#include "src/gui/windows/objectEditor/ObjectEditorPane.h"
#include "src/concurrent/task/Task.h"
#include "src/db/task/AddObjectTask.h"
#include "src/db/task/UpdateObjectTask.h"
#include "src/gui/windows/TaskProgressDialog.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h"
#include "src/db/DataStorage.h"

/*
 * Improvements:
 *   - when creating, set the focus to the first control not yet given (e. g. club)
 */

/*
 * Reasons to use a template based approach rather than, for example, with a
 * factory pattern:
 *   - no central place where all types are listed - the specialization is in
 *     the respective class file
 *   - it also works for creating a new object, when there is no object to pass
 *     to a factory
 */

/**
 * Requirements for T:
 *   - static QString objectTypeDescription ();
 *   - XXX
 */
template<class T> class ObjectEditorWindow: public ObjectEditorWindowBase
{
	public:
		// Construction
		ObjectEditorWindow (Mode mode, DataStorage &dataStorage, QWidget *parent=0, Qt::WindowFlags flags=0);
		virtual ~ObjectEditorWindow ();

		// Invocation
		// TODO: don't allow changing the registration/person name/...
		static db_id createObject (QWidget *parent, DataStorage &dataStorage);
		static void displayObject (QWidget *parent, DataStorage &dataStorage, const T &object);
		static int editObject (QWidget *parent, DataStorage &dataStorage, const T &object);

		// Database
		bool writeToDatabase (const T &object);

		// GUI events
		virtual void on_okButton_clicked ();

		db_id getId () const { return id; }

	private:
		ObjectEditorPane<T> *editorPane;
		Mode mode;
		db_id id;

};

// ******************
// ** Construction **
// ******************

template<class T> ObjectEditorWindow<T>::ObjectEditorWindow (Mode mode, DataStorage &dataStorage, QWidget *parent, Qt::WindowFlags flags):
	ObjectEditorWindowBase (dataStorage, parent, flags),
	mode (mode)
{
	editorPane = ObjectEditorPane<T>::create (mode, dataStorage, ui.objectEditorPane);
	ui.objectEditorPane->layout ()->addWidget (editorPane);

	switch (mode)
	{
		case modeCreate:
			setWindowTitle (QString ("%1 erstellen").arg (T::objectTypeDescription ()));
			break;
		case modeEdit:
			setWindowTitle (QString ("%1 editieren").arg (T::objectTypeDescription ()));
			break;
		case modeDisplay:
			setWindowTitle (QString ("%1 anzeigen").arg (T::objectTypeDescription ()));
			ui.okButton->setVisible (false);
			ui.cancelButton->setText (QString::fromUtf8 ("&Schließen"));
			editorPane->setEnabled (false);
			break;
	}

	resize (sizeHint ());
}

template<class T> ObjectEditorWindow<T>::~ObjectEditorWindow ()
{
}


// ****************
// ** Invocation **
// ****************

// TODO: allow presetting the registration/name/..., probably by passing in a const T&
// TODO: registration/name/... not editable
template<class T> db_id ObjectEditorWindow<T>::createObject (QWidget *parent, DataStorage &dataStorage)
{
	ObjectEditorWindow<T> *w=new ObjectEditorWindow<T> (modeCreate, dataStorage, parent);
	w->setAttribute (Qt::WA_DeleteOnClose, true);

	if (w->exec ()==QDialog::Accepted)
		return w->getId ();
	else
		return invalid_id;
}

// TODO: this should probably take an ID instead of a T&
// TODO: only show a close button
template<class T> void ObjectEditorWindow<T>::displayObject (QWidget *parent, DataStorage &dataStorage, const T &object)
{
	ObjectEditorWindow<T> *w=new ObjectEditorWindow<T> (modeDisplay, dataStorage, parent);
	w->setAttribute (Qt::WA_DeleteOnClose, true);
	w->editorPane->objectToFields (object);
	w->exec ();
}

// TODO: this should probably take an ID instead of a T&
template<class T> int ObjectEditorWindow<T>::editObject (QWidget *parent, DataStorage &dataStorage, const T &object)
{
	ObjectEditorWindow<T> *w=new ObjectEditorWindow<T> (modeEdit, dataStorage, parent);
	w->setAttribute (Qt::WA_DeleteOnClose, true);
	w->editorPane->objectToFields (object);
	return w->exec ();
}


// **************
// ** Database **
// **************

template<class T> bool ObjectEditorWindow<T>::writeToDatabase (const T &object)
{
	// TODO error handling

	switch (mode)
	{
		case modeCreate:
		{
			AddObjectTask<T> task (dataStorage, object);

			dataStorage.addTask (&task);
			TaskProgressDialog::waitTask (this, &task);

			if (task.isCompleted ())
				id=task.getId ();

			return task.isCompleted ();
		} break;
		case modeEdit:
		{
			UpdateObjectTask<T> task (dataStorage, object);

			dataStorage.addTask (&task);
			TaskProgressDialog::waitTask (this, &task);

			return task.isCompleted ();
		} break;
		case modeDisplay:
			assert (false);
			break;
	}

	assert (false);
	return false;
}


// ****************
// ** GUI events **
// ****************

template<class T> void ObjectEditorWindow<T>::on_okButton_clicked ()
{
	// The OK button was pressed. Check and store the object.
	try
	{
		T object=editorPane->determineObject ();
		if (writeToDatabase (object))
			accept (); // Close the dialog
	}
	catch (ObjectEditorPaneBase::AbortedException &e)
	{
		// User aborted, do nothing
	}
}

#endif // OBJECTEDITORWINDOW_H
