#include "SkTable.h"

#include <cassert>

#include <QApplication>
#include <QKeyEvent>

#include "src/gui/widgets/SkTableItem.h"

SkTable::SkTable (QWidget *parent)
	:QTableWidget (parent)
	/*
	 * Constructs an SkTable instance.
	 * Parameters:
	 *   - parent, name: passed to the QTable constructor.
	 */
{
	setSelectionMode (QAbstractItemView::NoSelection);
}

void SkTable::simulate_key (int key)
	/*
	 * Act as if a given key had been pressed.
	 * Parameters:
	 *   - key: the key to simulate.
	 */
{
	QKeyEvent e (QEvent::KeyPress, key, 0, 0);
	keyPressEvent (&e);
}

void SkTable::keyPressEvent (QKeyEvent *e)
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

		case Qt::Key_F9: sortByColumn (currentColumn ()); break;
		case Qt::Key_T: if (e->state ()&Qt::ControlButton) sortByColumn (currentColumn ()); break;

		default:
			emit key (e->key ());
			e->ignore ();
			break;
	}

	if (e->key ()!=Qt::Key_Return)	// Hack, weil accept () nicht funktioniert
		QTableWidget::keyPressEvent (e);

	// Pfusch, weil MainWindow das Event auch braucht, weil das Hilfe->Info
	// nicht den Eingabefocus erhält.
	e->ignore ();
}

void SkTable::set_table_column (int column, QString title, QString sample)
	/*
	 * Sets the a Table header column to a given title and adjusts the width so
	 * that the title and a provided sample text are fully visible (sort of).
	 * Parameters:
	 *   - column: the column index to set.
	 *   - title: the title text to set.
	 *   - sample: the sample text.
	 */
{
	QFont normal_font=qApp->font ();
	QFont bold_font=normal_font; bold_font.setBold (true);

	int sample_width=QFontMetrics (normal_font).width (sample);
	int title_width=QFontMetrics (bold_font).width (title);

	int width;
	if (sample_width>title_width)
		width=sample_width;
	else
		width=title_width+8;	// Hard coded Table header margin width

	setColumn (column, title, width);
}




SkTableItem *SkTable::set_cell (int row, int col, const QString &text, QColor bg)
	/*
	 * Sets a cell to a given text and color.
	 * Parameters:
	 *   - row, col: the coordinates of the cell to set.
	 *   - text: the text to set.
	 *   - bg: the background color for the cell.
	 * Return value:
	 *   the newly created Table item.
	 */
{
	SkTableItem *ret;
//					// This should make it faster, but when using it, the cell
//					// contents are not redrawn until they get focus. This
//					// might be bug in QT. TODO
//					ret=(SkTableItem *)item (row, col);
//					if (ret)
//					{
//						ret->set_background (bg);
//						ret->setText (std2q (text));
//						ret->paint ();
//					}
//					else
//					{
		db_id old_id=0;
		SkTableItem *old_item=(SkTableItem *)item (row, col);
		if (old_item) old_id=old_item->id ();
		setItem (row, col, ret=new SkTableItem (text, bg));
		ret->set_id (old_id);
//					}
	return ret;
}


void SkTable::removeAllRows ()
{
	setRowCount (0);
}

void SkTable::removeRow (int row)
	/*
	 * Remove a row from the Table.
	 * Parameters:
	 *   - row: the row to remove.
	 */
{
	int r=currentRow ();
	int c=currentColumn ();
	QTableWidget::removeRow (row);
	// TODO what happens if r/c does not exist any more?
	setCurrentCell (r, c);
}



int SkTable::row_from_column_id (db_id id, int col)
	/*
	 * Finds the row where a given ID is saved in a given column.
	 * Parameters:
	 *   - id: the ID to search for.
	 *   - col: the column to search in.
	 */
{
	if (id<=0) return -1;
	for (int r=0; r<rowCount (); r++)
	{
		SkTableItem *i=(SkTableItem *)item (r, col);
		if (i && i->id ()==id) return r;
	}
	return -1;
}

db_id SkTable::id_from_cell (int row, int col)
	/*
	 * Finds the ID that is saved in a given cell.
	 * Parameters:
	 *   - row, col: the coordinates of the cell.
	 * Return value:
	 *   - the ID saved in the cell item, if it exists.
	 *   - 0 else.
	 */
{
	if (row<0) return invalid_id;
	if (row>=rowCount ()) return invalid_id;
	SkTableItem *id_item=(SkTableItem *)item (row, col);
	if (id_item==NULL) return invalid_id;
	return id_item->id ();
}


void SkTable::columnClicked (int c)
	/*
	 * A column header was clicked. Sort the Table by the corresponding column.
	 * Parameters:
	 *   see QT documentation.
	 */
{
	sortByColumn (c);
}


void SkTable::setColumn (int column, QString caption, int width)
{
	QTableWidgetItem *headerItem=horizontalHeaderItem (column);

	if (headerItem)
		headerItem->setText (caption);
	else
		setHorizontalHeaderItem(column, new QTableWidgetItem (caption));

	setColumnWidth (column, width);
}
