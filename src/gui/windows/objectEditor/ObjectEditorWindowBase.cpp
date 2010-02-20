#include "ObjectEditorWindowBase.h"

ObjectEditorWindowBase::ObjectEditorWindowBase (DataStorage &dataStorage, QWidget *parent, Qt::WindowFlags flags):
	QDialog (parent, flags),
	dataStorage (dataStorage)
{
	ui.setupUi (this);
}

ObjectEditorWindowBase::~ObjectEditorWindowBase ()
{

}

