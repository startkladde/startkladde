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

bool yesNoQuestion (QWidget *parent, QString title, QString question)
{
	// TODO: Yes/No, but with ESC
	QMessageBox::StandardButtons buttons=QMessageBox::Ok | QMessageBox::Cancel;
	QMessageBox::StandardButton result=QMessageBox::question (parent, title, question, buttons, QMessageBox::Ok);
	return result==QMessageBox::Ok;
}

bool confirmProblem (QWidget *parent, const QString title, const QString problem)
	/*
	 * Displays a message and ask the user if he wishes to accept anyway.
	 * Parameters:
	 *   - parent: passed on to the QMessageBox constructor.
	 *   - msg: the message.
	 * Return value:
	 *   - if the user accepted.
	 */
{
	// TODO: Buttons Yes/No, but with Esc handling
	QString question=problem+" Trotzdem akzeptieren?";
	return yesNoQuestion (parent, title, question);
}

bool confirmProblem (QWidget *parent, const QString problem)
{
	return confirmProblem (parent, "Warnung", problem);
}

