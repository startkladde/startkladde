#include "sk_text_box.h"

sk_text_box::sk_text_box (QWidget *parent, const char *name)/*{{{*/
	:QLineEdit (parent, name)
	/*
	 * Constructs an sk_text_box instance.
	 * Parameters:
	 *   - parent, name: passed on to the QLineEdit constructor.
	 */
{
	// Qt3:
	//setFocusPolicy (QWidget::StrongFocus);
	// Qt4:
	setFocusPolicy (Qt::StrongFocus);
}
/*}}}*/

void sk_text_box::focusInEvent (QFocusEvent *qfe)/*{{{*/
	/*
	 * Called when the input focus is set on the widget. Emits a signal.
	 * Parameters:
	 *   see the QT documentation.
	 */
{
	QLineEdit::focusInEvent (qfe);
	emit focus_in ();
}
/*}}}*/

void sk_text_box::focusOutEvent (QFocusEvent *qfe)/*{{{*/
	/*
	 * Called when the input focus is removed from the widget. Emits a signal.
	 * Parameters:
	 *   see the QT documentation.
	 */
{
	emit focus_out ();
	QLineEdit::focusOutEvent (qfe);
}
/*}}}*/
