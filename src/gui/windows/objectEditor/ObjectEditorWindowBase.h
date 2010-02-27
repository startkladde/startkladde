/*
 * ObjectEditorWindowBase.h
 *
 *  Created on: Aug 23, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTEDITORWINDOWBASE_H_
#define OBJECTEDITORWINDOWBASE_H_

#include <QtGui/QDialog>

#include "ui_ObjectEditorWindowBase.h"

namespace Db { namespace Cache { class Cache; } }

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

		ObjectEditorWindowBase (Db::Cache::Cache &cache, QWidget *parent=NULL, Qt::WindowFlags flags=0);
		virtual ~ObjectEditorWindowBase ();

	public slots:
		virtual void on_okButton_clicked ()=0;

	protected:
		Ui::ObjectEditorWindowBaseClass ui;
		Db::Cache::Cache &cache;
};


#endif
