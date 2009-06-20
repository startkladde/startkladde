#include "sk_combo_box.h"
#include <iostream>
#include <QtCore/QtDebug>
#include <QtGui/QCompleter>

/**
  * Constructs a sk_combo_box instance.
  * Parameters:
  *   - rw, parent, name: passsed on to the QComboBox constructor.
  */
sk_combo_box::sk_combo_box (bool editable, QWidget *parent)
	:QComboBox (parent)
{
	setInsertPolicy (QComboBox::NoInsert);
	setEditable (editable);
}

void sk_combo_box::setAutoCompletion (bool autocomplete) {
	if (autocomplete) {
		QCompleter* completer = this->completer();
		if (completer) {
        		completer->setCaseSensitivity (Qt::CaseInsensitive);
        		completer->setCompletionMode (QCompleter::PopupCompletion);
        		setCompleter (completer);
		}
	}
	else
		setCompleter (NULL);
}

/**
  * Inserts an Item. Overloaded function taking a std::string.
  * Parameters:
  *   - s, index: passed on to QComboBox::insertItem.
  */
void sk_combo_box::insertItem (const string &s, int index)
{
	QComboBox::insertItem (std2q (s), index);
}

string sk_combo_box::edit_text_string ()/*{{{*/
	/*
	 * Returns a string with the current contents of the editor field.
	 * Return value:
	 *   the contents of the editor field.
	 */
{
	// TODO: q2std
//	return string (q2std (lineEdit ()->text ()));
	return q2std (currentText ());
}/*}}}*/

string sk_combo_box::current_text_string ()/*{{{*/
	/*
	 * Returns a string with the current item.
	 * Return value:
	 *   the item.
	 */
{
	return string (q2std (currentText ()));
}/*}}}*/

/**
  * Fills the box with an array of strings.
  * Parameters:
  *   - array: the array of strings.
  *   - num: the number of entries in the array.
  *   - del: whether to delete the array.
  */
void sk_combo_box::fillStringArray (string **array, int num, bool del)
{
	hide ();
	clear ();
	for (int i=0; i<num; i++)
	{
		insertItem (*(array[num]));
		if (del) delete array[num];
	}
	if (del) if (num>0) delete[] array;
	show ();
}

void sk_combo_box::setCurrentItem (int index)/*{{{*/
	/*
	 * Sets the current item and emit the activation signal.
	 * Parameters:
	 *   - index: the index of the item to be made current.
	 */
{
	QComboBox::setCurrentItem (index);
	emit activated (index);
}/*}}}*/

void sk_combo_box::insert_if_new (const string &t)/*{{{*/
	/*
	 * Insert an item to the list if it is new.
	 * Parameters:
	 *   - t: the text of the new item.
	 */
{
	// TODO das sollte besser gehen. Insbesondere müsste das sortierbar sein.
	//
	
	bool is_new=true;
	for (int i=0; i<count (); i++)
	{
		if (t==q2std (text (i))) is_new=false;
	}

	if (is_new)
	{
		// Wenn die Liste leer war, wird offenbar das Textfeld überschrieben
		QString old_string=currentText ();
		insertItem (t);
		setCurrentText (old_string);
	}
}/*}}}*/

void sk_combo_box::cursor_to_end ()/*{{{*/
	/*
	 * Moves the cursor to the end of the editing field.
	 */
{
	QLineEdit *le=lineEdit ();
	if (le) le->setSelection (le->text ().length (), 0);
}
/*}}}*/

void sk_combo_box::focusOutEvent (QFocusEvent *event)/*{{{*/
{
	QComboBox::focusOutEvent (event);
	emit focus_out ();
}
/*}}}*/

void sk_combo_box::focusInEvent (QFocusEvent *event)/*{{{*/
{
	QComboBox::focusInEvent (event);
	emit focus_in ();
}
/*}}}*/
