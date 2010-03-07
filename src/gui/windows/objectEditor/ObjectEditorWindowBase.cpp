#include "ObjectEditorWindowBase.h"

#include "src/db/DbManager.h"

ObjectEditorWindowBase::ObjectEditorWindowBase (DbManager &manager, QWidget *parent, Qt::WindowFlags flags):
	QDialog (parent, flags),
	manager (manager),
	cache (manager.getCache ())
{
	ui.setupUi (this);
}

ObjectEditorWindowBase::~ObjectEditorWindowBase ()
{

}

