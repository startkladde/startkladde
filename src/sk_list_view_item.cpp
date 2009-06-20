#include "sk_list_view_item.h"

sk_list_view_item::sk_list_view_item (QListView *list, QListViewItem *after, QString text)/*{{{*/
	:QListViewItem (list, after, text)
{

}/*}}}*/

sk_list_view_item::sk_list_view_item (QListView *list, QListViewItem *after)/*{{{*/
	:QListViewItem (list, after)
{

}/*}}}*/

sk_list_view_item::sk_list_view_item (QListView *list, QString text)/*{{{*/
	:QListViewItem (list, text)
{

}/*}}}*/

