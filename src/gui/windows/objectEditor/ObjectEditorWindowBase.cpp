#include "ObjectEditorWindowBase.h"

ObjectEditorWindowBase::ObjectEditorWindowBase (Db::Cache::Cache &cache, QWidget *parent, Qt::WindowFlags flags):
	QDialog (parent, flags),
	cache (cache)
{
	ui.setupUi (this);
}

ObjectEditorWindowBase::~ObjectEditorWindowBase ()
{

}

