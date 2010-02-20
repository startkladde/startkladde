#include "dialogs.h"

// TODO: this is sometimes used where an error or an info should be displayed instead;
void showWarning (const QString &title, const QString &text, QWidget *parent)
	/*
	 * Displays a warning dialog with the given text.
	 * Parameters:
	 *   - text: the text to display.
	 *   - parent: the parent widget, passed on to the dialog constructor.
	 */
{
	QMessageBox::warning (parent, title, text, QMessageBox::Ok, QMessageBox::NoButton);
}
