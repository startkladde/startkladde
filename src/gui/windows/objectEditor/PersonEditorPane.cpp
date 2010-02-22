#include "PersonEditorPane.h"

#include "src/db/dataStorage/DataStorage.h"
#include "src/model/Person.h"
#include "src/text.h"

/*
 * TODO: disallow person name changes; allow merges only
 */

// ******************
// ** Construction **
// ******************

PersonEditorPane::PersonEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent):
	ObjectEditorPane<Person> (mode, dataStorage, parent)
{
	ui.setupUi(this);

	fillData ();

	ui.lastNameInput->setFocus ();
}

PersonEditorPane::~PersonEditorPane()
{

}

template<> ObjectEditorPane<Person> *ObjectEditorPane<Person>::create (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent)
{
	return new PersonEditorPane (mode, dataStorage, parent);
}


// ***********
// ** Setup **
// ***********

void PersonEditorPane::fillData ()
{
	// Clubs
	ui.clubInput->addItems (dataStorage.getClubs ());
	ui.clubInput->setCurrentText ("");
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PersonEditorPane::objectToFields (const Person &person)
{
	originalId=person.getId ();

	ui.lastNameInput->setText (person.nachname);
	ui.firstNameInput->setText (person.vorname);
	ui.clubInput->setCurrentText (person.club);
	ui.commentsInput->setText (person.comments);
}

Person PersonEditorPane::determineObject ()
{
	Person person;

	person.setId (originalId);

	person.nachname=ui.lastNameInput->text ();
	person.vorname=ui.firstNameInput->text ();
	person.club=ui.clubInput->currentText ();
	person.comments=ui.commentsInput->text ();

	// Error checks

	if (eintrag_ist_leer (person.nachname))
		errorCheck ("Es wurde kein Nachname angegeben.",
			ui.lastNameInput);

	if (eintrag_ist_leer (person.vorname))
		errorCheck ("Es wurde kein Vorname angegeben.",
			ui.firstNameInput);


	return person;
}
