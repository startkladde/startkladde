#include "dialogs.h"

void show_warning (const QString &text, QWidget *parent)/*{{{*/
	/*
	 * Displays a warning dialog with the given text.
	 * Parameters:
	 *   - text: the text to display.
	 *   - parent: the parent widget, passed on to the dialog constructor.
	 */
{
	QMessageBox::warning (parent, "Warnung", text, QMessageBox::Ok, QMessageBox::NoButton);
}/*}}}*/


