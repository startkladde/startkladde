/*
 * ObjectEditorPane.h
 *
 *  Created on: Aug 22, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTEDITORPANE_H_
#define OBJECTEDITORPANE_H_

#include <QtGui/QWidget>

#include "src/db/dbId.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required fro ObjectEditorWindowBase::Mode

class DataStorage;

/**
 * A Q_OBJECT base for the template ObjectEditorPane so we can use signals and
 * slots.
 */
class ObjectEditorPaneBase: public QWidget
{
	public:
		// Types
		class AbortedException: public std::exception {};

		ObjectEditorPaneBase (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent=NULL);
		virtual ~ObjectEditorPaneBase ();

	protected:
		virtual void errorCheck (const QString &problem, QWidget *widget);
		virtual void requiredField (const QString &value, QWidget *widget, const QString &problem);

		DataStorage &dataStorage;
		ObjectEditorWindowBase::Mode mode;
};

/**
 * A QWidget with editor fields used to edit an object.
 *
 * This slightly complex design is to allow the creating the editor pane for
 * individual types with Designer.
 *
 * To implement this class, inherit from an instantiation of this template and
 * implement the abstract methods. You should also specialize the create<T>
 * method template, like this:
 *   template<> ObjectEditorPane<Foo> *ObjectEditorPane<Foo>::create (QWidget *parent) {
 *     return new PlaneEditorPane (parent);
 *   }
 * If the create method template is not specialized, the implementation can
 * still be used, but its type must be explicitly named; template classes such
 * as ObjectEditorWindow<T> use the create method template so they don't have
 * to be specialized.
 */
template<class T> class ObjectEditorPane: public ObjectEditorPaneBase
{
	public:
		ObjectEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent=NULL);
		virtual ~ObjectEditorPane ();

		/**
		 * Implementations of this method must also store the original ID.
		 * @param object
		 */
		virtual void objectToFields (const T &object)=0;

		/**
		 * Creates an object, using the values from the input fields. Throws
		 * AbortedException if the user aborts.
		 * Implementations of this method must also restore the original ID.
		 */
		virtual T determineObject ()=0;

		/** @brief Implementations of ObjectEditorPane should specialize this template method */
		static ObjectEditorPane<T> *create (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent=NULL);

	protected:
		dbId originalId;

};

// *************************************
// ** ObjectEditorPane implementation **
// *************************************

template<class T> ObjectEditorPane<T>::ObjectEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent):
	ObjectEditorPaneBase (mode, dataStorage, parent),
	originalId (invalidId)
{
}

template<class T> ObjectEditorPane<T>::~ObjectEditorPane ()
{
}

#endif
