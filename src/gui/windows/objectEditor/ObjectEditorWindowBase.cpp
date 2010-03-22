#include "ObjectEditorWindowBase.h"

#include "src/db/DbManager.h"

ObjectEditorWindowBase::ObjectEditorWindowBase (DbManager &manager, QWidget *parent, Qt::WindowFlags flags):
	QDialog (parent, flags),
	manager (manager)
{
	ui.setupUi (this);
}

ObjectEditorWindowBase::~ObjectEditorWindowBase ()
{

}

