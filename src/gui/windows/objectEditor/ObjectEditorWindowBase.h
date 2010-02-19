/*
 * ObjectEditorWindowBase.h
 *
 *  Created on: Aug 23, 2009
 *      Author: mherrman
 */

#ifndef OBJECTEDITORWINDOWBASE_H_
#define OBJECTEDITORWINDOWBASE_H_

#include <QtGui/QDialog>

#include "ui_ObjectEditorWindowBase.h"

class DataStorage;


/**
 * Base class for ObjectEditorWindow, because templates cannot be Q_OBJECTs
 * TODO document for all, also: what goes into the base class and what not:
 *   - signals/slots must be declared in base
 *   - non-template stuff should be declared in base because it does not have
 *     to be in the header
 *   - template stuff must be declared in the template
 */
class ObjectEditorWindowBase: public QDialog
{
	Q_OBJECT

	public:
		// Types
		enum Mode { modeCreate, modeEdit, modeDisplay };

		ObjectEditorWindowBase (DataStorage &dataStorage, QWidget *parent=NULL, Qt::WindowFlags flags=0);
		virtual ~ObjectEditorWindowBase ();

	public slots:
		virtual void on_okButton_clicked ()=0;

	protected:
		Ui::ObjectEditorWindowBaseClass ui;
		DataStorage &dataStorage;
};


#endif /* OBJECTEDITORWINDOWBASE_H_ */
