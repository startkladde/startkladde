#include "SkTextBox.h"

SkTextBox::SkTextBox (QWidget *parent, const char *name)
	:QLineEdit (parent, name)
	/*
	 * Constructs an SkTextBox instance.
	 * Parameters:
	 *   - parent, name: passed on to the QLineEdit constructor.
	 */
{
	setFocusPolicy (Qt::StrongFocus);
}

void SkTextBox::focusInEvent (QFocusEvent *qfe)
	/*
	 * Called when the input focus is set on the widget. Emits a signal.
	 * Parameters:
	 *   see the QT documentation.
	 */
{
	QLineEdit::focusInEvent (qfe);
	emit focus_in ();
}

void SkTextBox::focusOutEvent (QFocusEvent *qfe)
	/*
	 * Called when the input focus is removed from the widget. Emits a signal.
	 * Parameters:
	 *   see the QT documentation.
	 */
{
	emit focus_out ();
	QLineEdit::focusOutEvent (qfe);
}
