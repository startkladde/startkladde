#include "PlaneEditorPane.h"

#include <QMessageBox>

#include "src/text.h"
#include "src/db/DataStorage.h"
#include "src/model/Plane.h"

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

PlaneEditorPane::PlaneEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent):
	ObjectEditorPane<Plane> (mode, dataStorage, parent)
{
	ui.setupUi(this);

	fillData ();

	ui.registrationInput->setFocus ();

	ui.registrationInput->setText (Plane::defaultRegistrationPrefix ());
//	ui.registrationInput->setCursorPosition (ui.registrationInput->text ().length ());
//	ui.registrationInput->end (false);
}

PlaneEditorPane::~PlaneEditorPane()
{

}

template<> ObjectEditorPane<Plane> *ObjectEditorPane<Plane>::create (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent)
{
	return new PlaneEditorPane (mode, dataStorage, parent);
}


// ***********
// ** Setup **
// ***********

void PlaneEditorPane::fillData ()
{
	// Categories
	ui.categoryInput->addItem ("", Plane::categoryNone);
	const QList<Plane::Category> categories=Plane::listCategories (false);
	for (int i=0; i<categories.size (); ++i)
		ui.categoryInput->addItem (Plane::categoryText (categories.at (i)), categories.at (i));
	ui.categoryInput->setCurrentItemByItemData (Plane::categoryNone);

	// Types
	ui.typeInput->addItems (dataStorage.getPlaneTypes ());
	ui.typeInput->setCurrentText ("");

	// Clubs
	ui.clubInput->addItems (dataStorage.getClubs ());
	ui.clubInput->setCurrentText ("");
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
	originalId=plane.getId ();

	ui.registrationInput->setText (plane.registration);
	ui.competitionIdInput->setText (plane.competitionId);
	ui.categoryInput->setCurrentItemByItemData (plane.category);
	ui.typeInput->setCurrentText (plane.type);
	ui.clubInput->setCurrentText (plane.club);
	ui.seatsInput->setValue (plane.numSeats);
	ui.commentsInput->setText (plane.comments);
}

Plane PlaneEditorPane::determineObject ()
{
	// TODO: checks go here; throw AbortedException if aborted
	Plane plane;

	plane.setId (originalId);

	plane.registration=ui.registrationInput->text ();
	plane.competitionId=ui.competitionIdInput->text ();
	plane.category=(Plane::Category)ui.categoryInput->currentItemData ().toInt ();
	plane.type=ui.typeInput->currentText ();
	plane.club=ui.clubInput->currentText ();
	plane.numSeats=ui.seatsInput->value ();
	plane.comments=ui.commentsInput->text ();


	// Error checks

	if (mode==ObjectEditorWindowBase::modeCreate && id_valid (dataStorage.getPlaneIdByRegistration (plane.registration)))
	{
		QString message=QString ("Es gibt bereits ein Flugzeug mit dem Kennzeichen %1").arg (plane.registration);
		QMessageBox::critical (this, "Flugzeug existiert bereits", message, QMessageBox::Ok, QMessageBox::NoButton);
		throw AbortedException ();
	}

	if (eintrag_ist_leer (plane.registration))
		errorCheck ("Es wurde kein Kennzeichen angegeben.",
			ui.registrationInput);

	if (plane.category==Plane::categoryNone)
		errorCheck ("Es wurde keine Gattung angegeben.",
			ui.categoryInput);

	Plane::Category registrationCategory=Plane::categoryFromRegistration (plane.registration);
	if (registrationCategory!=Plane::categoryNone && plane.category!=Plane::categoryNone && plane.category!=registrationCategory)
		errorCheck ("Die angegebene Gattung passt nicht zum Kennzeichen.",
			ui.categoryInput);

	if (eintrag_ist_leer (plane.type))
		errorCheck ("Es wurde kein Typ angegeben.",
			ui.typeInput);

	if (plane.numSeats<0)
		errorCheck ("Es wurde keine Sitzanzahl angegeben.",
			ui.seatsInput);

	if (plane.numSeats==0)
		errorCheck ("Es wurden 0 Sitze angegeben.",
			ui.seatsInput);

	int maxSeats=Plane::categoryMaxSeats (plane.category);
	if (maxSeats>=0 && plane.numSeats>maxSeats)
		errorCheck (QString::fromUtf8 ("Es wurden zu viele Sitze für die gewählte Gattung angegeben."),
			ui.seatsInput);


	return plane;
}
