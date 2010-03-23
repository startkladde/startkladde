#include "ObjectSelectWindowBase.h"

ObjectSelectWindowBase::ObjectSelectWindowBase (QWidget *parent):
	QDialog (parent),
	resultId (invalidId),
	newItem (NULL), unknownItem (NULL)
{
	ui.setupUi (this);
	setModal (true);
}

ObjectSelectWindowBase::~ObjectSelectWindowBase ()
{
//	QObject::connect (ui.objectList, SIGNAL (itemActivated (QTreeWidgetItem *, int)), this, SLOT (slot_double_click (QTreeWidgetItem *, int)));

}

dbId ObjectSelectWindowBase::getResultId () const
{
	return resultId;
}

void ObjectSelectWindowBase::on_objectList_itemActivated (QTreeWidgetItem *item, int column)
{
	(void)column;
	if (item) accept ();
}
