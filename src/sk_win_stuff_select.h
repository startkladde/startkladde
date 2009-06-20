#ifndef sk_win_stuff_select_h
#define sk_win_stuff_select_h

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlayout.h>

// Qt3:
//#include <qvbox.h>
//#include <qhbox.h>
// Qt4:
#include <Qt3Support>
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator

#include "data_types.h"
#include "sk_dialog.h"
#include "sk_list_view_item.h"
#include "stuff.h"

using namespace std;

enum selection_result { sr_cancelled, sr_ok, sr_new, sr_unknown, sr_none_selected };

/*
 * The helper classes for using slots and signals with the template class
 * sk_win_stuff_select.
 */
class selector_base:public sk_dialog/*{{{*/
{
	friend class selector_helper;

	public:
		selector_base (QWidget *parent, const char *name=NULL, bool modal=false, WFlags f=0)
			:sk_dialog (parent, name, modal, f) {};
	
		// Problem when there is a TYPE as parameter to one of our redirected slots.
		virtual void slot_ok ()=0;
		virtual void slot_double_click (QListViewItem *, const QPoint &, int)=0;
};
/*}}}*/

class selector_helper:public QObject/*{{{*/
{
	Q_OBJECT

	public:
		selector_helper (selector_base *_helped) { helped=_helped; }

	public slots:
		void slot_ok () { helped->slot_ok (); }
		void slot_double_click (QListViewItem *item, const QPoint &point, int column) { helped->slot_double_click (item, point, column); }
		void reject () { helped->reject (); }

	private:
		selector_base *helped;
};
/*}}}*/



/*
 * A dialog for letting the user select a stuff from a list. A "new" and
 * "unknown" entry are also given.
 * Usage:
 * Call do_selection, passing the stuff list.
 * If the result is sr_ok, you can use get_result_id () to get the selected ID.
 */

template<class TYPE> class sk_win_stuff_select:public selector_base/*{{{*/
{
	// A template class cannot be a Q_OBJECT

	public:
		sk_win_stuff_select (QWidget *parent, const char *name=NULL, WFlags f=0);
		~sk_win_stuff_select ();
		void test ();
		virtual selection_result do_selection (QString, QString, QPtrList<TYPE> &, db_id preselected=invalid_id);
		static string selection_result_text (selection_result sr);
		db_id get_result_id ();
		selector_helper *helper () { return _helper; }
	
	private:
		QLabel *text;
		QListView *list;
		QPushButton *but_ok;
		QPushButton *but_cancel;
		sk_list_view_item *new_item;
		sk_list_view_item *unknown_item;
		db_id result_id;
	
	private:
		int setup_columns ();
		void set_entry (sk_list_view_item *item, TYPE *entry, int num_columns);
		selector_helper *_helper;

//	private slots:
		void slot_ok ();
		void slot_double_click (QListViewItem *, const QPoint &, int);
};
/*}}}*/

template<class TYPE> sk_win_stuff_select<TYPE>::sk_win_stuff_select (QWidget *parent, const char *name, WFlags f)/*{{{*/
	:selector_base (parent, name, true, f)
	/*
	 * Creates a sk_win_stuff_select class.
	 * Parameters:
	 *   - parent, name, f: passed to the base class constructor.
	 */
{
	// Initialize variables/*{{{*/
	_helper=new selector_helper (this);
	result_id=invalid_id;
/*}}}*/

	// Create the controls/*{{{*/
	text=new QLabel ("", this, "text");
	list=new QListView (this, "list");
	but_ok=new QPushButton ("&OK", this, "but_ok");
	but_cancel=new QPushButton ("&Abbrechen", this, "but_cancel");
/*}}}*/

	// Setup the controls/*{{{*/
	list->setAllColumnsShowFocus (true);
	list->setSorting (-1);
	list->setSelectionMode (QListView::Single);
	but_ok->setDefault (true);
/*}}}*/

	// Connect the signals/*{{{*/
	QObject::connect (but_ok, SIGNAL (clicked ()), helper (), SLOT (slot_ok ()));
	QObject::connect (but_cancel, SIGNAL (clicked ()), helper (), SLOT (reject ()));
	QObject::connect (list, SIGNAL (doubleClicked (QListViewItem *, const QPoint&, int)), helper (), SLOT (slot_double_click (QListViewItem *, const QPoint &, int)));
/*}}}*/

	// Arrange the controls/*{{{*/
	QVBoxLayout *main_layout=new QVBoxLayout (this, 4, -1, "main_layout");
	main_layout->addWidget (text);
	main_layout->addWidget (list);

	QHBoxLayout *button_layout=new QHBoxLayout (main_layout, -1, "button_layout");
	button_layout->addStretch ();
	button_layout->addWidget (but_ok);
	button_layout->addWidget (but_cancel);
/*}}}*/
}/*}}}*/

template<class TYPE> sk_win_stuff_select<TYPE>::~sk_win_stuff_select ()/*{{{*/
	/*
	 * Cleans up a sk_win_stuff_select class.
	 */
{
	delete _helper;
}
/*}}}*/

template<class TYPE> void sk_win_stuff_select<TYPE>::slot_ok ()/*{{{*/
	/*
	 * The OK button was pressed. Close the dialog, accepting.
	 */
{
	accept ();
}
/*}}}*/

template<class TYPE> void sk_win_stuff_select<TYPE>::slot_double_click (QListViewItem *it, const QPoint &, int)/*{{{*/
	/*
	 * The list was double clicked.
	 */

{
	if (it) slot_ok ();
}
/*}}}*/

template<class TYPE> string sk_win_stuff_select<TYPE>::selection_result_text (selection_result sr)/*{{{*/
	/*
	 * Creates a text describing a selection_result.
	 * Parameters:
	 *   - sr: the selection_result.
	 * Return value:
	 *   - the description.
	 */
{
	switch (sr)
	{
		case sr_cancelled: return "cancelled";
		case sr_ok: return "OK";
		case sr_new: return "new";
		case sr_unknown: return "unknown";
		case sr_none_selected: return "none selected";
		default: log_error ("Unhandled selection_result in sk_win_stuff_select::selection_result_text ()"); return "?";
	}
}
/*}}}*/

template<class TYPE> db_id sk_win_stuff_select<TYPE>::get_result_id ()/*{{{*/
	/*
	 * Gets the selection result.
	 * Return value:
	 *   - the result.
	 */
{
	return result_id;
}
/*}}}*/



template<class TYPE> int sk_win_stuff_select<TYPE>::setup_columns ()/*{{{*/
	/*
	 * Sets up the list columns.
	 * Return value:
	 *   - the number of columns in the list.
	 */
{
	int i=0;
	string title;
	while (title=TYPE::get_selector_caption (i), !title.empty ())
	{
		list->addColumn (std2q (title));
		i++;
	}

	return i;
}
/*}}}*/

template<class TYPE> void sk_win_stuff_select<TYPE>::set_entry (sk_list_view_item *item, TYPE *entry, int num_columns)/*{{{*/
	/*
	 * Writes an entry to the list.
	 * Parameters:
	 *   - item: the list item where to write to.
	 *   - entry: the entry to write.
	 *   - num_columns: the number of columns to write.
	 */
{
	for (int i=0; i<num_columns; i++)
	{
		item->setText (i, std2q (entry->get_selector_value (i)));
	}
}
/*}}}*/

template<class TYPE> selection_result sk_win_stuff_select<TYPE>::do_selection (QString caption_text, QString label_text, QPtrList<TYPE> &stuff_list, db_id preselected)/*{{{*/
	/*
	 * Displays the selector.
	 * Parameters:
	 *   - caption_text: the text to display in the window title.
	 *   - label_text: the text displayed in the descriptive label.
	 *   - stuff_list: the list to select from.
	 *   - preselected: the ID to preselect, or 0 if none should be
	 *     preselected.
	 * Return value:
	 *   - the selection result.
	 */
{
	setCaption (caption_text);
	text->setText (label_text);
	if (label_text.isEmpty ()) text->hide ();

	int num_columns=list->columns ();
	for (int i=0; i<num_columns; i++) list->removeColumn (0);

	num_columns=setup_columns ();

	sk_list_view_item *last_item;
	unknown_item=last_item=new sk_list_view_item (list, "(Unbekannt)");
	new_item=last_item=new sk_list_view_item (list, last_item, "(Neu anlegen)");

	list->setSelected (unknown_item, true);

	for (QPtrListIterator<TYPE> it (stuff_list); *it; ++it)
	{
		last_item=new sk_list_view_item (list, last_item);
		last_item->id=(*it)->id;
		set_entry (last_item, *it, num_columns);
		if (!id_invalid (preselected) && (*it)->id==preselected) list->setSelected (last_item, true);
	}

	int result=exec ();

	if (result==QDialog::Rejected)
	{
		return sr_cancelled;
	}
	else
	{
		QListViewItem *current_item=list->selectedItem ();
		if (!current_item)
			return sr_none_selected;
		else if (current_item==new_item)
			return sr_new;
		else if (current_item==unknown_item)
			return sr_unknown;
		else
		{
			// There may only be sk_list_view_items in the list.
			result_id=((sk_list_view_item *)current_item)->id;
			return sr_ok;
		}
	}
}
/*}}}*/

#endif

