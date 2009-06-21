#ifndef sk_list_view_item_h
#define sk_list_view_item_h

/*
 * sk_list_view_item
 * martin
 * 2004-10-18
 */

#include <QListView>
#include <QString>
// XXX
#include <Qt3Support>
#define QListView Q3ListView
#define QListViewItem Q3ListViewItem

#include "src/db/db_types.h"

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

