#include "sk_table.h"
#include <qapplication.h>

sk_table::sk_table (QWidget *parent, const char *name)/*{{{*/
	:QTable (parent, name)
	/*
	 * Constructs an sk_table instance.
	 * Parameters:
	 *   - parent, name: passed to the QTable constructor.
	 */
{
	setSelectionMode (QTable::NoSelection);
}/*}}}*/

void sk_table::simulate_key (int key)/*{{{*/
	/*
	 * Act as if a given key had been pressed.
	 * Parameters:
	 *   - key: the key to simulate.
	 */
{
	QKeyEvent e (QEvent::KeyPress, key, 0, 0);
	keyPressEvent (&e);
}/*}}}*/

void sk_table::keyPressEvent (QKeyEvent *e)/*{{{*/
	/*
	 * Handle a key pressed.
	 * This implements, among others, HJKL.
	 * Parameters:
	 *   - e: the event describing the key.
	 */
{
	switch (e->key ())
	{
		case Qt::Key_H: simulate_key (Qt::Key_Left); break;
		case Qt::Key_J: simulate_key (Qt::Key_Down); break;
		case Qt::Key_K: simulate_key (Qt::Key_Up); break;
		case Qt::Key_L: simulate_key (Qt::Key_Right); break;

		case Qt::Key_Return: emit key (Qt::Key_Return); break;
		case Qt::Key_Insert: emit key (Qt::Key_Insert); break;
		case Qt::Key_Delete: emit key (Qt::Key_Delete); break;

		case Qt::Key_F9: sortColumn (currentColumn (), true, true); break;
		case Qt::Key_T: if (e->state ()&Qt::ControlButton) sortColumn (currentColumn (), true, true); break;

		default:
			emit key (e->key ());
			e->ignore ();
			break;
	}
	
	if (e->key ()!=Qt::Key_Return)	// Hack, weil accept () nicht funktioniert
		QTable::keyPressEvent (e);

	// Pfusch, weil sk_win_main das Event auch braucht, weil das Hilfe->Info
	// nicht den Eingabefocus erhält.
	e->ignore ();
}/*}}}*/

void sk_table::set_table_column (QHeader *header, int column, string title, string sample)/*{{{*/
	/*
	 * Sets the a table header column to a given title and adjusts the width so
	 * that the title and a provided sample text are fully visible (sort of).
	 * Parameters:
	 *   - header: the table header to use. Use QTable::horizontalHeader ().
	 *   - column: the column index to set.
	 *   - title: the title text to set.
	 *   - sample: the sample text.
	 */
{
	QFont normal_font=qApp->font ();
	QFont bold_font=normal_font; bold_font.setBold (true);

	int sample_width=QFontMetrics (normal_font).width (std2q (sample));
	int title_width=QFontMetrics (bold_font).width (std2q (title));

	int width;
	if (sample_width>title_width)
		width=sample_width;
	else
		width=title_width+8;	// Hard coded table header margin width

	header->setLabel (column, std2q (title), width);

}
/*}}}*/




sk_table_item *sk_table::set_cell (int row, int col, const string &text, QColor bg)/*{{{*/
	/*
	 * Sets a cell to a given text and color.
	 * Parameters:
	 *   - row, col: the coordinates of the cell to set.
	 *   - text: the text to set.
	 *   - bg: the background color for the cell.
	 * Return value:
	 *   the newly created table item.
	 */
{
	sk_table_item *ret;
//					// This should make it faster, but when using it, the cell
//					// contents are not redrawn until they get focus. This
//					// might be bug in QT. TODO
//					ret=(sk_table_item *)item (row, col);
//					if (ret)
//					{
//						ret->set_background (bg);
//						ret->setText (std2q (text));
//						ret->paint ();
//					}
//					else
//					{
		db_id old_id=0;
		sk_table_item *old_item=(sk_table_item *)item (row, col);
		if (old_item) old_id=old_item->id ();
		setItem (row, col, ret=new sk_table_item (this, text, bg));
		ret->set_id (old_id);
//					}
	return ret;
}/*}}}*/

sk_table_item *sk_table::set_cell (int row, int col, const QString &text, QColor bg)/*{{{*/
	/*
	 * An overloaded function taking a QString instead of a std::string.
	 */
{
	return set_cell (row, col, q2std (text), bg);
}/*}}}*/

sk_table_item *sk_table::set_cell (int row, int col, const char *text, QColor bg)/*{{{*/
	/*
	 * An overloaded function taking a char* instead of a std::string.
	 */
{
	return set_cell (row, col, string (text), bg);
}/*}}}*/



void sk_table::clear_table ()/*{{{*/
	/*
	 * Clears the table.
	 */
{
	int n=numRows ();
	if (n==0) return;

	// Use a QMemArray because this is faster (than clear ()? Huh???).
	// TODO: check this.
	QMemArray<int> f(n);
	for (int i=0; i<n; i++) f[i]=i;
	removeRows (f);
}/*}}}*/



void sk_table::removeRow (int row)/*{{{*/
	/*
	 * Remove a row from the table.
	 * Parameters:
	 *   - row: the row to remove.
	 */
{
	int r=currentRow ();
	int c=currentColumn ();
	QTable::removeRow (row);
	// TODO what happens if r/c does not exist any more?
	setCurrentCell (r, c);
}/*}}}*/



int sk_table::row_from_column_id (db_id id, int col)/*{{{*/
	/*
	 * Finds the row where a given ID is saved in a given column.
	 * Parameters:
	 *   - id: the ID to search for.
	 *   - col: the column to search in.
	 */
{
	if (id<=0) return -1;
	for (int r=0; r<numRows (); r++)
	{
		sk_table_item *i=(sk_table_item *)item (r, col);
		if (i && i->id ()==id) return r;
	}
	return -1;
}/*}}}*/

db_id sk_table::id_from_cell (int row, int col)/*{{{*/
	/*
	 * Finds the ID that is saved in a given cell.
	 * Parameters:
	 *   - row, col: the coordinates of the cell.
	 * Return value:
	 *   - the ID saved in the cell item, if it exists.
	 *   - 0 else.
	 */
{
	if (row<0) return 0;
	if (row>=numRows ()) return 0;
	sk_table_item *id_item=(sk_table_item *)item (row, col);
	if (id_item==NULL) return 0;
	return id_item->id ();
}
/*}}}*/


void sk_table::columnClicked (int c)/*{{{*/
	/*
	 * A column header was clicked. Sort the table by the corresponding column.
	 * Parameters:
	 *   see QT documentation.
	 */
{
	sortColumn (c, true, true);
}/*}}}*/

