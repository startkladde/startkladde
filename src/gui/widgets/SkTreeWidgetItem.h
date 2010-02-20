#ifndef _SkTreeWidgetItem_h
#define _SkTreeWidgetItem_h

#include <QTreeWidget>
#include <QString>

// TODO reduce dependencies
#include "src/db/dbId.h"

class SkTreeWidgetItem:public QTreeWidgetItem
{
	public:
		SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after, QString text);
		SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after);
		SkTreeWidgetItem (QTreeWidget *parent, QString text);

		db_id id;
};

#endif

