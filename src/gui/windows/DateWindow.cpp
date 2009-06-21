#include "DateWindow.h"

#include <QLayout>

void DateWindow::init_common ()/*{{{*/
	/*
	 * Does initialization stuff common to all of the constructors.
	 * Initializes the widgets and connects signals.
	 */
{
	but_ok=new QPushButton ("&OK", this);
	but_cancel=new QPushButton ("&Abbrechen", this);

	QObject::connect (but_ok, SIGNAL (clicked ()), this, SLOT (slot_ok ()));
	QObject::connect (but_cancel, SIGNAL (clicked ()), this, SLOT (slot_cancel ()));

	lbl_datum=new QLabel ("&Datum:", this, "lbl_datum", 0);
//	lbl_datum->adjustSize (); lbl_datum->resize (lbl_datum->width (), widget_height);
	lbl_zeit=new QLabel ("&Zeit:", this, "lbl_zeit", 0);
//	lbl_zeit->adjustSize (); lbl_zeit->resize (lbl_zeit->width (), widget_height);

	edit_datum=new QDateEdit (this); //edit_datum->resize (120, widget_height);
	edit_zeit=new QTimeEdit (this); //edit_zeit->resize (120, widget_height);

	//edit_datum->setOrder (QDateEdit::DMY);
	//edit_datum->setSeparator (".");
	//edit_datum->setOrder (QDateEdit::YMD);
	//edit_datum->setSeparator ("-");
	//edit_datum->setAutoAdvance (true);

	lbl_datum->setBuddy (edit_datum);
	lbl_zeit->setBuddy (edit_zeit);

	QGridLayout *layout_edit=new QGridLayout (this, 0, 2, widget_border, widget_gap, "layout_edit");
	layout_edit->addWidget (lbl_datum, 0, 0);
	layout_edit->addWidget (edit_datum, 0, 1);
	layout_edit->addWidget (lbl_zeit, 1, 0);
	layout_edit->addWidget (edit_zeit, 1, 1);
	layout_edit->setRowStretch (2, 1);

	QHBoxLayout *layout_buttons=new QHBoxLayout ();
	layout_buttons->addStretch (1);
	layout_buttons->addWidget (but_ok);
	layout_buttons->addWidget (but_cancel);
	layout_edit->addMultiCellLayout (layout_buttons, 3, 3, 0, 1);


//	edit_zeit->setOrder (QDateEdit::DMY);
//	edit_zeit->setSeparator (".");

	setCaption ("Zeit einstellen");
}/*}}}*/

DateWindow::DateWindow (QWidget *parent, QDate datum)/*{{{*/
	:QDialog (parent)
	/*
	 * Constructs an instance and write a given date to the editor field.
	 * Parameters:
	 *   - datum: the date to set.
	 */
{
	init_common ();
	edit_datum->setDate (datum);
}/*}}}*/

DateWindow::DateWindow (QWidget *parent, QDateTime dt)/*{{{*/
	:QDialog (parent)
	/*
	 * Constructs an instance and write a given date and time to the editor
	 * fields.
	 * Parameters:
	 *   - dt: the datetime to set.
	 */
{
	init_common ();
	edit_datum->setDate (dt.date ());
	edit_zeit->setTime (dt.time ());
}/*}}}*/

void DateWindow::edit_date ()/*{{{*/
	/*
	 * Displays the window for editing the date.
	 */
{
	// TODO use QLayout
	setMinimumSize (320, edit_datum->height ()+but_cancel->height ()+3*8);
	edit_datum->setFocus ();
	edit_datum->show ();
	edit_zeit->hide ();
	lbl_datum->show ();
	lbl_zeit->hide ();

	exec ();
}/*}}}*/

void DateWindow::edit_datetime ()/*{{{*/
	/*
	 * Displays the window for editing the date and time.
	 */
{
	// TODO Use QLayout
	setMinimumSize (320, edit_datum->height ()+edit_zeit->height ()+but_cancel->height ()+4*8);
	edit_datum->show ();
	edit_zeit->show ();
	lbl_datum->show ();
	lbl_zeit->show ();
	edit_datum->setFocus ();

	exec ();
}/*}}}*/


void DateWindow::slot_ok ()/*{{{*/
	/*
	 * The OK button was pressed. Emit the signals and close the window.
	 */
{
	emit date_accepted (edit_datum->date ());
	emit datetime_accepted (QDateTime (edit_datum->date (), edit_zeit->time ()));
	accept ();
}/*}}}*/

void DateWindow::slot_cancel ()/*{{{*/
	/*
	 * The cancel button was pressed. Close the window.
	 */
{
	reject ();
}/*}}}*/

int DateWindow::exec ()/*{{{*/
	/*
	 * Displays the dialog.
	 */
{
	resize (180, edit_zeit->height ()+edit_datum->height ()+but_ok->height ()+4*8);
	resizeEvent (NULL);
	return QDialog::exec ();
}/*}}}*/

