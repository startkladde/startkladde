#ifndef _EntitySelectWindow_h
#define _EntitySelectWindow_h

#include <QPushButton>
#include <QLabel>
#include <QListView>
#include <QLayout>
#include <QTreeWidget>
#include <QDialog>

// TODO reduce dependencies
#include "src/gui/widgets/SkTreeWidgetItem.h"
#include "src/model/Entity.h"
#include "src/logging/messages.h"

enum selection_result { sr_cancelled, sr_ok, sr_new, sr_unknown, sr_none_selected };

// TODO setRootDecorated (false)
// TODO alternatingRowColors

/*
 * The helper classes for using slots and signals with the template class
 * EntitySelectWindow.
 */
class selector_base:public QDialog
{
	friend class selector_helper;

	public:
		selector_base (QWidget *parent, const char *name=NULL, bool modal=false, Qt::WindowFlags f=0)
			:QDialog (parent, name, modal, f) {};

		// Problem when there is a TYPE as parameter to one of our redirected slots.
		virtual void slot_ok ()=0;
		virtual void slot_double_click (QTreeWidgetItem *, int)=0;
};

class selector_helper:public QObject
{
	Q_OBJECT

	public:
		selector_helper (selector_base *_helped) { helped=_helped; }

	public slots:
		void slot_ok () { helped->slot_ok (); }
		void slot_double_click (QTreeWidgetItem *item, int column) { helped->slot_double_click (item, column); }
		void reject () { helped->reject (); }

	private:
		selector_base *helped;
};



/*
 * A dialog for letting the user select a Entity from a list. A "new" and
 * "unknown" entry are also given.
 * Usage:
 * Call do_selection, passing the Entity list.
 * If the result is sr_ok, you can use get_result_id () to get the selected ID.
 */

template<class TYPE> class EntitySelectWindow:public selector_base
{
	// A template class cannot be a Q_OBJECT

	public:
		EntitySelectWindow (QWidget *parent, const char *name=NULL, Qt::WindowFlags f=0);
		~EntitySelectWindow ();
		void test ();
		virtual selection_result do_selection (QString, QString, QList<TYPE> &, db_id preselected=invalid_id);
		static QString selection_result_text (selection_result sr);
		db_id get_result_id ();
		selector_helper *helper () { return _helper; }

	private:
		QLabel *text;
		QTreeWidget *list;
		QPushButton *but_ok;
		QPushButton *but_cancel;
		SkTreeWidgetItem *new_item;
		SkTreeWidgetItem *unknown_item;
		db_id result_id;

	private:
		int setup_columns ();
		void set_entry (SkTreeWidgetItem *item, const TYPE &entry, int num_columns);
		selector_helper *_helper;

//	private slots:
		void slot_ok ();
		void slot_double_click (QTreeWidgetItem *, int);
};

template<class TYPE> EntitySelectWindow<TYPE>::EntitySelectWindow (QWidget *parent, const char *name, Qt::WindowFlags f)
	:selector_base (parent, name, true, f)
	/*
	 * Creates a EntitySelectWindow class.
	 * Parameters:
	 *   - parent, name, f: passed to the base class constructor.
	 */
{
	// Initialize variables
	_helper=new selector_helper (this);
	result_id=invalid_id;

	// Create the controls
	text=new QLabel ("", this, "text");
	list=new QTreeWidget (this);
	but_ok=new QPushButton ("&OK", this, "but_ok");
	but_cancel=new QPushButton ("&Abbrechen", this, "but_cancel");

	// Setup the controls
	list->setAllColumnsShowFocus (true);
	list->setSortingEnabled (false);
	list->setSelectionMode (QAbstractItemView::SingleSelection);
	but_ok->setDefault (true);

	// Connect the signals
	QObject::connect (but_ok, SIGNAL (clicked ()), helper (), SLOT (slot_ok ()));
	QObject::connect (but_cancel, SIGNAL (clicked ()), helper (), SLOT (reject ()));
	QObject::connect (list, SIGNAL (itemActivated (QTreeWidgetItem *, int)), helper (), SLOT (slot_double_click (QTreeWidgetItem *, int)));

	// Arrange the controls
	QVBoxLayout *main_layout=new QVBoxLayout (this, 4, -1, "main_layout");
	main_layout->addWidget (text);
	main_layout->addWidget (list);

	QHBoxLayout *button_layout=new QHBoxLayout (main_layout, -1, "button_layout");
	button_layout->addStretch ();
	button_layout->addWidget (but_ok);
	button_layout->addWidget (but_cancel);
}

template<class TYPE> EntitySelectWindow<TYPE>::~EntitySelectWindow ()
	/*
	 * Cleans up a EntitySelectWindow class.
	 */
{
	delete _helper;
}

template<class TYPE> void EntitySelectWindow<TYPE>::slot_ok ()
	/*
	 * The OK button was pressed. Close the dialog, accepting.
	 */
{
	accept ();
}

template<class TYPE> void EntitySelectWindow<TYPE>::slot_double_click (QTreeWidgetItem *it, int)
	/*
	 * The list was double clicked.
	 */

{
	if (it) slot_ok ();
}

template<class TYPE> QString EntitySelectWindow<TYPE>::selection_result_text (selection_result sr)
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
		default: log_error ("Unhandled selection_result in EntitySelectWindow::selection_result_text ()"); return "?";
	}
}

template<class TYPE> db_id EntitySelectWindow<TYPE>::get_result_id ()
	/*
	 * Gets the selection result.
	 * Return value:
	 *   - the result.
	 */
{
	return result_id;
}



template<class TYPE> int EntitySelectWindow<TYPE>::setup_columns ()
	/*
	 * Sets up the list columns.
	 * Return value:
	 *   - the number of columns in the list.
	 */
{
	int i=0;
	QString title;

	QStringList header;

	// TODO should probably use an ObjectModel
	while (title=TYPE::get_selector_caption (i), !title.isEmpty ())
	{
		header.append (title);
		++i;
	}

	list->setColumnCount(header.size ());
	list->setHeaderLabels (header);

	return i;
}

template<class TYPE> void EntitySelectWindow<TYPE>::set_entry (SkTreeWidgetItem *item, const TYPE &entry, int num_columns)
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
		item->setText (i, entry.get_selector_value (i));
	}
}

template<class TYPE> selection_result EntitySelectWindow<TYPE>::do_selection (QString caption_text, QString label_text, QList<TYPE> &entityList, db_id preselected)
	/*
	 * Displays the selector.
	 * Parameters:
	 *   - caption_text: the text to display in the window title.
	 *   - label_text: the text displayed in the descriptive label.
	 *   - entityList: the list to select from.
	 *   - preselected: the ID to preselect, or 0 if none should be
	 *     preselected.
	 * Return value:
	 *   - the selection result.
	 */
{
	setCaption (caption_text);
	text->setText (label_text);
	if (label_text.isEmpty ()) text->hide ();

	list->setColumnCount (0);

	int num_columns=setup_columns ();

	SkTreeWidgetItem *last_item;
	unknown_item=last_item=new SkTreeWidgetItem (list, "(Unbekannt)");
	new_item=last_item=new SkTreeWidgetItem (list, last_item, "(Neu anlegen)");

	list->setCurrentItem (unknown_item);

	foreach (const TYPE &it, entityList)
	{
		last_item=new SkTreeWidgetItem (list, last_item);
		last_item->id=it.getId ();
		set_entry (last_item, it, num_columns);
		if (!id_invalid (preselected) && it.getId ()==preselected)
			list->setCurrentItem (last_item);
	}

	int result=exec ();

	if (result==QDialog::Rejected)
	{
		return sr_cancelled;
	}
	else
	{
//		QTreeWidgetItem *current_item=list->selectedItem ();
		QList<QTreeWidgetItem *> selected=list->selectedItems ();
		if (selected.empty ())
			return sr_none_selected;
		else if (selected[0]==new_item)
			return sr_new;
		else if (selected[0]==unknown_item)
			return sr_unknown;
		else
		{
			// There may only be sk_list_view_items in the list.
			result_id=((SkTreeWidgetItem *)selected[0])->id;
			return sr_ok;
		}
	}
}

#endif

