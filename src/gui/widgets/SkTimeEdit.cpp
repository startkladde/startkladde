#include "SkTimeEdit.h"

#include <QLayout>

// Class management
SkTimeEdit::SkTimeEdit (QWidget *parent, const char *name)
	:QFrame (parent, name)
	/*
	 * Constructs a sk_time_edit window instance.
	 * Parameters:
	 *   - parent, name: passed to the parent class constructor.
	 */
{
	invert=false;

	cbox=new QCheckBox (this, "cbox");
	tedit=new QTimeEdit (this);
	pbut=new QPushButton (this, "pbug");

	QHBoxLayout *layout=new QHBoxLayout (this, 0, -1, "layout");
	layout->add (cbox);
	layout->add (tedit);
	layout->add (pbut);

	// OLDVERS: N�chste Zeile weg
	reset ();

	QObject::connect (pbut, SIGNAL (clicked ()), this, SIGNAL (clicked ()));
	QObject::connect (cbox, SIGNAL (clicked ()), this, SLOT (update_cbox ()));
	QObject::connect (cbox, SIGNAL (clicked ()), this, SIGNAL (cbox_clicked ()));
	QObject::connect (tedit, SIGNAL (timeChanged (const QTime &)), this, SIGNAL (time_changed ()));

//	resizeEvent (NULL);
}

void SkTimeEdit::set_mode (sk_te_mode p_mode)
	/*
	 * Sets the editor mode. There are the following modes:
	 *   - tm_time: time editor only
	 *   - tm_box_time: a checkbox on the left and a time editor on the right
	 *     (depending on the value of the check box)
	 *   - tm_time_but: a time editor on the left and a button on the right
	 * Parameters:
	 *   - p_mode: the mode to set
	 */
{
	// It would be nice to have eache subwidget be focused on tab, like
	// multiple focus proxies.
	mode=p_mode;

	switch (mode)
	{
		case tm_time:
			set_checked (false);
			cbox->hide ();
			tedit->show ();
			pbut->hide ();
			setFocusProxy (tedit);
			break;
		case tm_box_time:
			cbox->show ();
			setFocusProxy (tedit);
			update_cbox ();
			pbut->hide ();
			break;
		case tm_time_but:
			set_checked (false);
			cbox->hide ();
			tedit->show ();
			pbut->show ();
			setFocusProxy (tedit);
			break;
		default:
			log_error ("Unhandled mode in sk_time_edit::set_mode ()");
	}
//	resizeEvent (NULL);
}

void SkTimeEdit::reset ()
	/*
	 * Resets the window to an initial state.
	 */
{
	tedit->setDisplayFormat ("hh:mm");
	set_time (QTime (0, 0, 0, 0));
	set_mode (tm_time);
	set_checked (true);
}


// Slots
void SkTimeEdit::set_current_time ()
	/*
	 * Writes the current time to the time editor.
	 */
{
	// TODO time zone?
	tedit->setTime (QTime::currentTime ());
}


// Time editor widget
QTime SkTimeEdit::time ()
	/*
	 * Gets the time from the time editor.
	 * Return value:
	 *   - the time
	 */
{
	return tedit->time ();
}

void SkTimeEdit::set_time (const QTime &t)
	/*
	 * Writes a given time to the time editor.
	 * Parameters:
	 *   - t: the time to write
	 */
{
	// MURX: W�hrend die Zeit gesetzt wird, das Signal disconnecten, damit nur
	// �nderungen von der UI ein Signal ausl�sen.
	// TODO: Timezone?
	QObject::disconnect (tedit, SIGNAL (timeChanged (const QTime &)), this, SIGNAL (time_changed ()));
	tedit->setTime (t);
	QObject::connect (tedit, SIGNAL (timeChanged (const QTime &)), this, SIGNAL (time_changed ()));
}

void SkTimeEdit::null_zeit ()
	/*
	 * Sets the time editor to a null time.
	 */
{
	QTime t;
	tedit->setTime (t);
}



// Checkbox
void SkTimeEdit::set_cbox_text (const QString &t)
	/*
	 * Sets the text of the check box label.
	 * Parameters:
	 *   - t: the new text.
	 */
{
	cbox->setText (t);
}

void SkTimeEdit::set_checked (bool c)
	/*
	 * Sets the checked property of the check box.
	 * Parameters:
	 *   - c: the new value of the checked property.
	 */
{
	cbox->setChecked (c);
	update_cbox ();
}

bool SkTimeEdit::checked ()
	/*
	 * Returns the value of the checked property of the checkbox.
	 * Return value:
	 *   - the checked property.
	 */
{
	return cbox->isChecked ();
}

void SkTimeEdit::update_cbox ()
	/*
	 * Shows or hides the time editor field, depending on whether it should be
	 * visible.
	 */
{
	// TODO change the name of this function.
	if (time_enabled ())
	{
		tedit->show ();
		tedit->setEnabled (true);
		setFocusProxy (tedit);
	}
	else
	{
		tedit->setEnabled (false);
		tedit->hide ();
		setFocusProxy (cbox);
	}
}

bool SkTimeEdit::time_enabled ()
	/*
	 * Determines whether the time editor should be visible, depending on the
	 * mode and the checkbox.
	 * Return value:
	 *   - true if the editor should be visible.
	 *   - false else.
	 */
{
	if (mode==tm_box_time)
		return (cbox->isChecked ()==invert);
	else
		return true;
}


// Push button
void SkTimeEdit::set_pbut_text (const QString &t)
	/*
	 * Sets the text of the button.
	 * Parameters:
	 *   - t: the new text
	 */
{
	pbut->setText (t);
}


// Events
//void sk_time_edit::resizeEvent (QResizeEvent *e)
//	/*
//	 * Arranges the controls, depending on the mode.
//	 * Paremters:
//	 *   -e: the resize event given by the QT library.
//	 */
//{
//	// TODO use QLayout
//	switch (mode)
//	{
//		case tm_time:
//			tedit->setGeometry (0, 0, width (), height ());
//			break;
//		case tm_box_time:
//			cbox->resize (width ()/2-8, height ());
//			tedit->resize (width ()/2-8, height ());
//			cbox->move (0, 0);
//			tedit->move (width ()-tedit->width (), 0);
//			break;
//		case tm_time_but:
//			tedit->resize (width ()/2-8, height ());
//			pbut->resize (width ()/2-8, height ());
//			tedit->move (0, 0);
//			pbut->move (width ()-pbut->width (), 0);
//			break;
//		default:
//			log_error ("Unhandled mode in sk_time_edit::resizeEvent ()");
//	}
//
//}

