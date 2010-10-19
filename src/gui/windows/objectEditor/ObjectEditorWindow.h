#ifndef OBJECTEDITORWINDOW_H
#define OBJECTEDITORWINDOW_H

#include <cassert>

// TODO many dependencies in header, maybe move to .cpp and instantiate
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h"
#include "src/gui/windows/objectEditor/ObjectEditorPane.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/util/qString.h"
#include "src/db/DbManager.h"


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
		ObjectEditorWindow (Mode mode, DbManager &manager, QWidget *parent=0, Qt::WindowFlags flags=0);
		virtual ~ObjectEditorWindow ();

		// Invocation
		static dbId createObject (QWidget *parent, DbManager &manager);
		static dbId createObject (QWidget *parent, DbManager &manager, const T &nameObject);
		static void displayObject (QWidget *parent, DbManager &manager, const T &object);
		static int editObject (QWidget *parent, DbManager &manager, const T &object);

		// Database
		bool writeToDatabase (T &object);

		// GUI events
		virtual void on_okButton_clicked ();

		dbId getId () const { return id; }

	protected:
		ObjectEditorPane<T> *editorPane;
		Mode mode;
		dbId id;

};


// ******************
// ** Construction **
// ******************

template<class T> ObjectEditorWindow<T>::ObjectEditorWindow (Mode mode, DbManager &manager, QWidget *parent, Qt::WindowFlags flags):
	ObjectEditorWindowBase (manager, parent, flags),
	mode (mode)
{
	editorPane = ObjectEditorPane<T>::create (mode, manager.getCache (), ui.objectEditorPane);
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
			ui.cancelButton->setText (utf8 ("&Schließen"));
			editorPane->setEnabled (false);
			break;
	}

	resize (sizeHint ());
}

template<class T> ObjectEditorWindow<T>::~ObjectEditorWindow ()
{
		std::cout << "ObjectEditorWindow being deleted" << std::endl;
}


// ****************
// ** Invocation **
// ****************

// TODO: allow presetting the registration/name/..., probably by passing in a const T&
// TODO: registration/name/... not editable
template<class T> dbId ObjectEditorWindow<T>::createObject (QWidget *parent, DbManager &manager)
{
	// Note that we cannot use WA_DeleteOnClose here because we need to read the
	// ID from w after it has been closed.
	ObjectEditorWindow<T> w (modeCreate, manager, parent);

	if (w.exec ()==QDialog::Accepted)
		return w.getId ();
	else
		return invalidId;
}

template<class T> dbId ObjectEditorWindow<T>::createObject (QWidget *parent, DbManager &manager, const T &nameObject)
{
	// Note that we cannot use WA_DeleteOnClose here because we need to read the
	// ID from w after it has been closed.
	ObjectEditorWindow<T> w (modeCreate, manager, parent);

	w.editorPane->setNameObject (nameObject);

	if (w.exec ()==QDialog::Accepted)
		return w.getId ();
	else
		return invalidId;
}

// TODO: this should probably take an ID instead of a T&
// TODO: only show a close button
template<class T> void ObjectEditorWindow<T>::displayObject (QWidget *parent, DbManager &manager, const T &object)
{
	ObjectEditorWindow<T> *w=new ObjectEditorWindow<T> (modeDisplay, manager, parent);
	w->setAttribute (Qt::WA_DeleteOnClose, true);
	w->editorPane->objectToFields (object);
	w->exec ();
}

// TODO: this should probably take an ID instead of a T&
template<class T> int ObjectEditorWindow<T>::editObject (QWidget *parent, DbManager &manager, const T &object)
{
	ObjectEditorWindow<T> *w=new ObjectEditorWindow<T> (modeEdit, manager, parent);
	w->setAttribute (Qt::WA_DeleteOnClose, true);
	w->editorPane->objectToFields (object);
	return w->exec ();
}


// **************
// ** Database **
// **************

template<class T> bool ObjectEditorWindow<T>::writeToDatabase (T &object)
{
	switch (mode)
	{
		case modeCreate:
		{
			try
			{
				std::cout << "Create object: " << object.toString () << std::endl;
				id=manager.createObject (object, this);
				std::cout << "after manager.createObject: id is " << id << " is " << getId () << std::endl;
				return true;
			}
			catch (OperationCanceledException)
			{
				// TODO the cache may now be inconsistent
				return false;
			}
		} break;
		case modeEdit:
		{
			try
			{
				std::cout << "Update object: " << object.toString () << std::endl;
				manager.updateObject (object, this);
				return true;
			}
			catch (OperationCanceledException)
			{
				// TODO the cache may now be inconsistent
				return false;
			}
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
		{
			std::cout << "after writeToDatabase, right before accept, id is " << getId () << std::endl;
			accept (); // Close the dialog
		}
	}
	catch (ObjectEditorPaneBase::AbortedException &e)
	{
		// User aborted, do nothing
	}
}

#endif // OBJECTEDITORWINDOW_H
