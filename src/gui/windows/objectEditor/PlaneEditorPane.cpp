#include "PlaneEditorPane.h"

#include <QMessageBox>

#include "src/text.h"
#include "src/db/cache/Cache.h"
#include "src/model/Plane.h"
#include "src/util/qString.h"
#include "src/notr.h"

/*
 * Improvements:
 *   - cursor to end of registration input on create
 */

/*
 * TODO remake:
 *   - autocompletion type consistent in all editors
 *   - disallow plane registration changes
 */



// ******************
// ** Construction **
// ******************

PlaneEditorPane::PlaneEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent):
	ObjectEditorPane<Plane> (mode, cache, parent)
{
	ui.setupUi(this);

	fillData ();


	ui.registrationInput->setText (Plane::defaultRegistrationPrefix ());
	ui.registrationInput->setFocus ();
//	ui.registrationInput->setCursorPosition (ui.registrationInput->text ().length ());
//	ui.registrationInput->end (false);
}

PlaneEditorPane::~PlaneEditorPane()
{

}

template<> ObjectEditorPane<Plane> *ObjectEditorPane<Plane>::create (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent)
{
	return new PlaneEditorPane (mode, cache, parent);
}


// ***********
// ** Setup **
// ***********

void PlaneEditorPane::fillData ()
{
	// Categories
	ui.categoryInput->addItem (notr (""), Plane::categoryNone);
	const QList<Plane::Category> categories=Plane::listCategories (false);
	for (int i=0; i<categories.size (); ++i)
		ui.categoryInput->addItem (Plane::categoryText (categories.at (i)), categories.at (i));
	ui.categoryInput->setCurrentItemByItemData (Plane::categoryNone);

	// Types
	ui.typeInput->addItem (notr (""));
	ui.typeInput->addItems (cache.getPlaneTypes ());
	ui.typeInput->setEditText (notr (""));

	// Clubs
	ui.clubInput->addItem (notr (""));
	ui.clubInput->addItems (cache.getClubs ());
	ui.clubInput->setEditText (notr (""));
}

void PlaneEditorPane::setNameObject (const Plane &nameObject)
{
	if (!isBlank (nameObject.registration))
	{
		ui.registrationInput->setText (nameObject.registration);
		ui.registrationInput->setEnabled (false);
		on_registrationInput_editingFinished ();
	}
}

// ****************
// ** GUI events **
// ****************

void PlaneEditorPane::on_registrationInput_editingFinished ()
{
	if (ui.categoryInput->currentItemData ()==Plane::categoryNone)
	{
		Plane::Category category=Plane::categoryFromRegistration (ui.registrationInput->text ());
		if (category!=Plane::categoryNone)
			ui.categoryInput->setCurrentItemByItemData (category);
	}
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PlaneEditorPane::objectToFields (const Plane &plane)
{
	ui.registrationInput->setText (plane.registration);
	ui.callsignInput->setText (plane.callsign);
	ui.categoryInput->setCurrentItemByItemData (plane.category);
	ui.typeInput->setEditText (plane.type);
	ui.clubInput->setEditText (plane.club);
	ui.seatsInput->setValue (plane.numSeats);
	ui.commentsInput->setText (plane.comments);
}

void PlaneEditorPane::fieldsToObject (Plane &plane)
{
	// TODO: checks go here; throw AbortedException if aborted

	plane.registration=ui.registrationInput->text ().simplified ();
	plane.callsign=ui.callsignInput->text ().simplified ();
	plane.category=(Plane::Category)ui.categoryInput->currentItemData ().toInt ();

	plane.type=ui.typeInput->currentText ().simplified ();
	plane.club=ui.clubInput->currentText ().simplified ();
	plane.numSeats=ui.seatsInput->value ();
	plane.comments=ui.commentsInput->text ().simplified ();


	// Error checks
	// TODO TR punctuation in errorCheck for all EditorPanes

	if (mode==ObjectEditorWindowBase::modeCreate && idValid (cache.getPlaneIdByRegistration (plane.registration)))
	{
		QString message=tr ("A plane with the registration %1 already exists").arg (plane.registration);
		QMessageBox::critical (this, tr ("Plane already exists"), message, QMessageBox::Ok, QMessageBox::NoButton);
		throw AbortedException ();
	}

	if (isNone (plane.registration))
		errorCheck (tr ("Registration not specified"),
			ui.registrationInput);

	if (plane.category==Plane::categoryNone)
		errorCheck (tr ("Category not specified"),
			ui.categoryInput);

	Plane::Category registrationCategory=Plane::categoryFromRegistration (plane.registration);
	if (registrationCategory!=Plane::categoryNone && plane.category!=Plane::categoryNone && plane.category!=registrationCategory)
		errorCheck (tr ("The selected category does not match the registration"),
			ui.categoryInput);

	if (isNone (plane.type))
		errorCheck (tr ("Model not specified"),
			ui.typeInput);

	if (plane.numSeats<0)
		errorCheck (tr ("Number of seats not specified"),
			ui.seatsInput);

	if (plane.numSeats==0)
		errorCheck (tr ("0 seats specified"),
			ui.seatsInput);

	int maxSeats=Plane::categoryMaxSeats (plane.category);
	if (maxSeats>=0 && plane.numSeats>maxSeats)
		errorCheck (tr ("To many seats specified for the selected category"),
			ui.seatsInput);
}
