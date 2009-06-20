#ifndef sk_list_view_item_h
#define sk_list_view_item_h

/*
 * sk_list_view_item
 * martin
 * 2004-10-18
 */
#include <qlistview.h>
#include <qstring.h>
#include "src/db/db_types.h"

// Qt4
#include <Qt3Support>
#define QListView Q3ListView
#define QListViewItem Q3ListViewItem

using namespace std;

class sk_list_view_item:public QListViewItem
{
	public:
		sk_list_view_item (QListView *list, QListViewItem *after, QString text);
		sk_list_view_item (QListView *list, QListViewItem *after);
		sk_list_view_item (QListView *list, QString text);

		db_id id;
};

#endif

