#ifndef _SkTreeWidgetItem_h
#define _SkTreeWidgetItem_h

/*
 * SkTreeWidgetItem
 * martin
 * 2004-10-18
 */

#include <QTreeWidget>
#include <QString>

#include "src/db/dbTypes.h"

class SkTreeWidgetItem:public QTreeWidgetItem
{
	public:
		SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after, QString text);
		SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after);
		SkTreeWidgetItem (QTreeWidget *parent, QString text);

		db_id id;
};

#endif

