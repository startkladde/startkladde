#include "LabelComboBox.h"

#include <QLayout>

#include "src/config/options.h"

lbl_cbox::lbl_cbox (const char *text, bool editable, QWidget *parent, const char *name)/*{{{*/
	:QFrame (parent, name)
	/*
	 * Creates a lbl_cbox instance.
	 * Parameters:
	 *   - text: the text show in the label.
	 *   - editable: whether the combobox is editable.
	 *   - parent, name: passed on to the QFrame constructor.
	 */
{
	focus_out_locked=false;

	cbox=new SkComboBox (editable, this);
	full_cbox=new SkComboBox (editable, this);
	lbl=new QLabel (cbox, text, this, "lbl");

	QHBoxLayout *lay_main=new QHBoxLayout (this, 0, -1, "lay_main");

	lay_main->add (lbl);
	lay_main->add (cbox);
	lay_main->add (full_cbox);

	lay_main->setStretchFactor (lbl, 0);
	lay_main->setStretchFactor (cbox, 1);
	lay_main->setStretchFactor (full_cbox, 1);

	QObject::connect (cbox, SIGNAL (activated (const QString &)), this, SIGNAL (activated (const QString &)));
	QObject::connect (cbox, SIGNAL (highlighted (const QString &)), this, SIGNAL (highlighted (const QString &)));
	QObject::connect (cbox, SIGNAL (focus_out ()), this, SLOT (some_focus_out ()));
	QObject::connect (full_cbox, SIGNAL (activated (const QString &)), this, SIGNAL (activated (const QString &)));
	QObject::connect (full_cbox, SIGNAL (highlighted (const QString &)), this, SIGNAL (highlighted (const QString &)));
	QObject::connect (full_cbox, SIGNAL (focus_out ()), this, SLOT (some_focus_out ()));

	// Die volle Liste ist gr�n, die andere rot (Debug-Feature)
	if (opts.colorful)
	{
		QPalette palette;

		lbl->setAutoFillBackground (true);

		palette=lbl      ->palette (); palette.setColor (QPalette::Background, QColor (127, 255, 255)); lbl      ->setPalette (palette);
		palette=cbox     ->palette (); palette.setColor (QPalette::Base      , QColor (255, 127, 127)); cbox     ->setPalette (palette);
		palette=full_cbox->palette (); palette.setColor (QPalette::Base      , QColor (127, 255, 127)); full_cbox->setPalette (palette);
	}

	used_cbox=NULL;
	set_active_cbox (full_cbox);
//	set_active_cbox (cbox);
}/*}}}*/

void lbl_cbox::set_active_cbox (SkComboBox *b)/*{{{*/
	/*
	 * Stellt die aktive cbox ein.
	 * Attention: the contents of the current editor field are copied to the new
	 * box, so the box should be switched before changing something (for example
	 * fillStringArrayDelete, which always operates on the partial box.
	 */
{
	QString old_string;
	bool focus=false;

	if (used_cbox)
	{
		old_string=used_cbox->currentText ();
		focus=used_cbox->hasFocus ();
	}

	cbox->hide ();
	full_cbox->hide ();

	used_cbox=b;
	used_cbox->show ();
	bool old_fol=focus_out_locked;
	focus_out_locked=true;
	if (focus) used_cbox->setFocus ();
	focus_out_locked=old_fol;
	setFocusProxy (used_cbox);
	used_cbox->setCurrentText (old_string);

	lbl->setBuddy (used_cbox);
}/*}}}*/

void lbl_cbox::use_full (bool full)/*{{{*/
	/*
	 * Specifies wheter the fully filled or the partially filled cbox is used.
	 * Attention: the contents of the current editor field are copied to the new
	 * box, so the box should be switched before changing something (for example
	 * fillStringArrayDelete, which always operates on the partial box.
	 */
{
	if (full)
		set_active_cbox (full_cbox);
	else
		set_active_cbox (cbox);
}/*}}}*/

//void lbl_cbox::resizeEvent (QResizeEvent *e)/*{{{*/
//	/*
//	 * The widget was resized. Change the size of the subwidgets accordingly.
//	 * Parameters:
//	 *   - e: not used.
//	 */
//{
//	lbl->setFixedHeight (height ());
//	cbox->setFixedHeight (height ());
//	cbox->setFixedWidth (width ()-lbl->width ());
//	full_cbox->setFixedHeight (height ());
//	full_cbox->setFixedWidth (width ()-lbl->width ());
//	// TODO call base class function?
//	QFrame::resizeEvent (e);
//}/*}}}*/

void lbl_cbox::setAutoCompletion (bool a)/*{{{*/
	/*
	 * Sets whether the cboxes use auto completion.
	 * Parameters:
	 *   - a: wheter to use autocompletion.
	 */
{
	cbox->setAutoCompletion (a);
	full_cbox->setAutoCompletion (a);
}/*}}}*/



// TODO remove del
void lbl_cbox::fillStringArray (SkComboBox *box, const QStringList& array, bool clear_first, bool blank_line, bool del)/*{{{*/
	/*
	 * Fills one of the cboxes with an array of strings.
	 * This is the main filling function which is called with different sets of
	 * parameters (see below).
	 * Parameters:
	 *   - box: the cbox to fill.
	 *   - array: the array with the strings.
	 *   - num: the number of strings in the array.
	 *   - clear_first: whether to clear the box before filling.
	 *   - blank_line: whether the list should contain an empty line.
	 *   - del: whether the array should be deleted afterwards.
	 */
{
	if (clear_first) box->clear ();
	if (blank_line) box->insertItem ("");

	box->insertStringList (array);

//	for (int i=0; i<num; i++)
//	{
//		if (i%11==0||i==num-1) emit progress (i, num-1);
//		box->insertItem (*(array[i]));
//	}

	if (array.count ()==1)
		box->setCurrentText (array[0]);
}/*}}}*/

void lbl_cbox::fillStringArray (const QStringList& array, bool clear_first, bool blank_line, bool del)/*{{{*/
	/*
	 * Fills the partial cbox with an array of strings.
	 * Parameters:
	 *   - array, num, clear_first, blank_line, del: passed on to
	 *     the main filling function.
	 */
{
	fillStringArray (cbox, array, clear_first, blank_line, del);
}/*}}}*/

void lbl_cbox::fillFullStringArray (const QStringList& array, bool clear_first, bool blank_line, bool del)/*{{{*/
	/*
	 * Fills the full cbox with an array of strings.
	 * Parameters:
	 *   - array, num, clear_first, blank_line, del: passed on to
	 *     the main filling function.
	 */
{
	fillStringArray (full_cbox, array, clear_first, blank_line, del);
}/*}}}*/

void lbl_cbox::insert_full_if_new (const QString &t)/*{{{*/
	/*
	 * Inserts an item into the full cbox if it is new.
	 * Parameters:
	 *   - t: the text for the item.
	 */
{
	full_cbox->insert_if_new (t);
}/*}}}*/


// The follwing functions copy other functions but use different
// parameters types (std::QString, QString, char *) for convenience.

QString lbl_cbox::current_text_string ()/*{{{*/
{
	return currentText ();
}/*}}}*/

// The following functions operate on both cboxes at the same time.
void lbl_cbox::setEditText (const QString &newText)/*{{{*/
	/*
	 * Sets the text of the editor field.
	 * Parameters:
	 *   - newText: the text to set.
	 */
{
	cbox->setEditText (newText);
	full_cbox->setEditText (newText);
}/*}}}*/

void lbl_cbox::setCurrentText (const QString &t)/*{{{*/
	/*
	 * Sets the current text.
	 * Parameters:
	 *   - t: the text to set.
	 */
{
	cbox->setCurrentText (t);
	full_cbox->setCurrentText (t);
}/*}}}*/



QString lbl_cbox::currentText () const/*{{{*/
	/*
	 * Reads the current text from the active field.
	 * Return value:
	 *   the current text.
	 */
{
	return used_cbox->currentText ();
}/*}}}*/

QString lbl_cbox::edit_text_string ()/*{{{*/
	/*
	 * Reads the current text from the active field.
	 * Return value:
	 *   the current text.
	 */
{
	return used_cbox->lineEdit ()->text ();
}/*}}}*/

void lbl_cbox::clear ()/*{{{*/
	/*
	 * Clears the partial cbox.
	 */
{
	cbox->clear ();
}/*}}}*/



void lbl_cbox::some_focus_out ()/*{{{*/
	/*
	 * One of the cboxes lost focus.
	 * This may also happen when the cboxes are switched, in which case the
	 * signal should not be emitted. Thus, the signal can be inhibited by
	 * focus_out_locked.
	 */
{
	if (!focus_out_locked) emit focus_out ();
}/*}}}*/

void lbl_cbox::select_all ()/*{{{*/
	/*
	 * Selects the complete text in the input field of the active cbox.
	 */
{
	// TODO: die used_cbox-Pruefung sollte auch an anderen Stellen durchgefuehrt
	// werden.
	if (used_cbox)
	{
		QLineEdit *le=used_cbox->lineEdit ();
		if (le)
		{
			le->selectAll ();
		}
	}
}/*}}}*/

