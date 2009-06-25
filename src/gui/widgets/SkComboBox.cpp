#include "SkComboBox.h"

#include <iostream>

#include <QCompleter>
#include <QtDebug> // XXX

/**
  * Constructs a SkComboBox instance.
  * Parameters:
  *   - rw, parent, name: passsed on to the QComboBox constructor.
  */
SkComboBox::SkComboBox (bool editable, QWidget *parent)
	:QComboBox (parent)
{
	setInsertPolicy (QComboBox::NoInsert);
	setEditable (editable);
}

void SkComboBox::setAutoCompletion (bool autocomplete) {
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
  * Inserts an Item. Overloaded function taking a std::QString.
  * Parameters:
  *   - s, index: passed on to QComboBox::insertItem.
  */
void SkComboBox::insertItem (const QString &s, int index)
{
	QComboBox::insertItem (s, index);
}

QString SkComboBox::edit_text_string ()
	/*
	 * Returns a QString with the current contents of the editor field.
	 * Return value:
	 *   the contents of the editor field.
	 */
{
	return currentText ();
}

QString SkComboBox::current_text_string ()
	/*
	 * Returns a QString with the current item.
	 * Return value:
	 *   the item.
	 */
{
	return currentText ();
}

/**
  * Fills the box with an array of strings.
  * Parameters:
  *   - array: the array of strings.
  *   - num: the number of entries in the array.
  *   - del: whether to delete the array.
  */
void SkComboBox::fillStringArray (QString **array, int num, bool del)
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

void SkComboBox::setCurrentItem (int index)
	/*
	 * Sets the current item and emit the activation signal.
	 * Parameters:
	 *   - index: the index of the item to be made current.
	 */
{
	QComboBox::setCurrentItem (index);
	emit activated (index);
}

void SkComboBox::insert_if_new (const QString &t)
	/*
	 * Insert an item to the list if it is new.
	 * Parameters:
	 *   - t: the text of the new item.
	 */
{
	// TODO das sollte besser gehen. Insbesondere m�sste das sortierbar sein.
	//

	bool is_new=true;
	for (int i=0; i<count (); i++)
	{
		if (t==text (i)) is_new=false;
	}

	if (is_new)
	{
		// Wenn die Liste leer war, wird offenbar das Textfeld �berschrieben
		QString old_string=currentText ();
		insertItem (t);
		setCurrentText (old_string);
	}
}

void SkComboBox::cursor_to_end ()
	/*
	 * Moves the cursor to the end of the editing field.
	 */
{
	QLineEdit *le=lineEdit ();
	if (le) le->setSelection (le->text ().length (), 0);
}

void SkComboBox::focusOutEvent (QFocusEvent *event)
{
	QComboBox::focusOutEvent (event);
	emit focus_out ();
}

void SkComboBox::focusInEvent (QFocusEvent *event)
{
	QComboBox::focusInEvent (event);
	emit focus_in ();
}
