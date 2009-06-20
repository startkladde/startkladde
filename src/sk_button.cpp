#include "sk_button.h"

sk_button::sk_button (db_id dt, const QString &text, QWidget *parent, const char *name)/*{{{*/
	:QPushButton (text, parent, name)
	/*
	 * Constructs an sk_button, given the additional data.
	 * Parameters:
	 *   - dt: the additional data to save
	 *   - text, parent, name: passed on to the QPushButton constructor.
	 */
{
	data=dt;
	QObject::connect (this, SIGNAL (clicked ()), this, SLOT (slot_clicked ()));
}/*}}}*/

void sk_button::slot_clicked ()/*{{{*/
	/*
	 * The button was clicked, emit the clicked signal passing the additional data.
	 */
{
	emit clicked (data);
}/*}}}*/

