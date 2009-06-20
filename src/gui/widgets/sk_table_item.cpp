#include "sk_table_item.h"

sk_table_item::sk_table_item (QTable *table, const QString &text, QColor bg)/*{{{*/
	:QTableItem (table, QTableItem::Never, text)
	/*
	 * Constructs an sk_table_item instance.
	 * Parameters:
	 *   - table, text: passed on to the QTableItem constructor.
	 *   - bg: the background color to use.
	 */
{
	background=bg;
	data_id=0;
	data=NULL;
}/*}}}*/

sk_table_item::sk_table_item (QTable *table, const string &text, QColor bg)/*{{{*/
	:QTableItem (table, QTableItem::Never, std2q (text))
	/*
	 * Overloaded function, using std::string instead of QString.
	 */
{
	background=bg;
	data_id=0;
	data=NULL;
}/*}}}*/

sk_table_item::sk_table_item (QTable *table, const char *text, QColor bg)/*{{{*/
	:QTableItem (table, QTableItem::Never, (QString)text)
	/*
	 * Overloaded function, using char* instead of QString.
	 */
{
	background=bg;
	data_id=0;
	data=NULL;
}/*}}}*/

void sk_table_item::paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)/*{{{*/
	/*
	 * Paints the table item with the correct background color.
	 * Parameters:
	 *   see QT documentation.
	 * Return value:
	 *   see QT documentation.
	 */
{
	QColorGroup g (cg);
//	g.setColor (QColorGroup::Base, QColor (255, 127, 127));
	g.setColor (QColorGroup::Base, background);
	g.setColor (QColorGroup::Text, QColor (0,0,0));
	QTableItem::paint (p, g, cr, selected);
}/*}}}*/


void sk_table_item::set_id (db_id i)/*{{{*/
	/*
	 * Sets the saved ID.
	 * Parameters:
	 *   - i: the ID to set.
	 */
{
	data_id=i;
}/*}}}*/

db_id sk_table_item::id ()/*{{{*/
	/*
	 * Gets the saved ID.
	 * Return value:
	 *   the ID.
	 */
{
	return data_id;
}/*}}}*/


void sk_table_item::set_background (QColor c)/*{{{*/
	/*
	 * Sets the background color.
	 * Parameters:
	 *   - c: the color.
	 */
{
	background=c;
}/*}}}*/

void sk_table_item::set_background (int red, int green, int blue)/*{{{*/
	/*
	 * Sets the backgound color, given the components.
	 * Parameters:
	 *   - red, green, blue: the color components.
	 */
{
	background.setRgb (red, green, blue);
}/*}}}*/
	
QColor sk_table_item::get_background ()/*{{{*/
	/*
	 * Gets the background color.
	 * Return value:
	 *   the color.
	 */
{
	return background;
}/*}}}*/

int sk_table_item::alignment () const/*{{{*/
	/*
	 * Returns the alignment for the cell.
	 * Return value:
	 *   - see the QT documentation.
	 */
{
	return Qt::AlignLeft;
}/*}}}*/

