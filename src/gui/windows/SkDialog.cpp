#include "SkDialog.h"


// TODO: es gibt noch eine andere Klasse, die im Konstruktor ein status_dialog
// bekommt. Sollte die vielleicht von dieser Klasse abgleietet werden?

SkDialog::SkDialog (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog)/*{{{*/
	:QDialog (parent, name, modal, f)
	/*
	 * Construct an SkDialog.
	 * Parameters:
	 *   - parent, name, modal, f: passed on to the QDialog constructor.
	 *   - status_dialog: the dialog (or whatever) to display status messages with.
	 */
{
	if (status_dialog)
	{
		QObject::connect (this, SIGNAL (status (QString)), status_dialog, SLOT (set_status (QString)));
		QObject::connect (this, SIGNAL (progress (int, int)), status_dialog, SLOT (set_progress (int, int)));
		QObject::connect (this, SIGNAL (long_operation_start ()), status_dialog, SLOT (show_splash ()));
		QObject::connect (this, SIGNAL (long_operation_end ()), status_dialog, SLOT (hide_splash ()));
	}
}/*}}}*/

void SkDialog::db_connect (QObject *ob)/*{{{*/
	/*
	 * Creates a connection for the db_change mechanism.
	 * Parameters:
	 *   - ob: the child object to connect with.
	 * TODO: diesen Mechanismus genauer dokumentieren.
	 */
{
	// Wenn ein Objekt die Datenbank ge�ndert hat, weiterreichen.
	QObject::connect (ob, SIGNAL (db_change (db_event *)), this, SIGNAL (db_change (db_event *)));
	// Wenn Datenbank�nderungen �bernommen werden sollen, weiterreichen.
	QObject::connect (this, SIGNAL (db_update (db_event *)), ob, SLOT (slot_db_update (db_event *)));
}/*}}}*/

void SkDialog::slot_db_update (db_event  *event)/*{{{*/
	/*
	 * Called by a signal _by the parent_ when a database change has happened.
	 * Emit the signal so children get notified.
	 */
{
	emit db_update (event);
}/*}}}*/

