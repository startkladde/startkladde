#ifndef OBJECTSELECTWINDOWBASE_H
#define OBJECTSELECTWINDOWBASE_H

#include <QtGui/QDialog>

#include "ui_ObjectSelectWindowBase.h"

#include "src/db/dbId.h"

class SkTreeWidgetItem;

class ObjectSelectWindowBase : public QDialog
{
	Q_OBJECT

	public:
		enum Result { resultCancelled, resultOk, resultNew, resultUnknown, resultNoneSelected };

		ObjectSelectWindowBase (QWidget *parent=NULL);
		~ObjectSelectWindowBase ();

		virtual dbId getResultId () const;

	protected slots:
		void on_objectList_itemActivated (QTreeWidgetItem *item, int column);

	protected:
		Ui::ObjectSelectWindowBaseClass ui;

		dbId resultId;
		SkTreeWidgetItem *newItem;
		SkTreeWidgetItem *unknownItem;
};

#endif
