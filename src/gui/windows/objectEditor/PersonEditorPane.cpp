#include "PersonEditorPane.h"

#include "src/db/cache/Cache.h"
#include "src/model/Person.h"
#include "src/text.h"

/*
 * TODO: disallow person name changes; allow merges only
 */

// ******************
// ** Construction **
// ******************

PersonEditorPane::PersonEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent):
	ObjectEditorPane<Person> (mode, cache, parent)
{
	ui.setupUi(this);

	fillData ();

	ui.lastNameInput->setFocus ();
}

PersonEditorPane::~PersonEditorPane()
{

}

template<> ObjectEditorPane<Person> *ObjectEditorPane<Person>::create (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent)
{
	return new PersonEditorPane (mode, cache, parent);
}


// ***********
// ** Setup **
// ***********

void PersonEditorPane::fillData ()
{
	// Clubs
	ui.clubInput->addItem ("");
	ui.clubInput->addItems (cache.getClubs ());
	ui.clubInput->setEditText ("");
}

void PersonEditorPane::setNameObject (const Person &nameObject)
{
	if (!isBlank (nameObject.lastName))
	{
		ui.lastNameInput->setText (nameObject.lastName);
		ui.lastNameInput->setEnabled (false);
	}

	if (!isBlank (nameObject.firstName))
	{
		ui.firstNameInput->setText (nameObject.firstName);
		ui.firstNameInput->setEnabled (false);
	}
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PersonEditorPane::objectToFields (const Person &person)
{
	originalId=person.getId ();

	ui.lastNameInput->setText (person.lastName);
	ui.firstNameInput->setText (person.firstName);
	ui.clubInput->setEditText (person.club);
	ui.commentsInput->setText (person.comments);
}

Person PersonEditorPane::determineObject ()
{
	Person person;

	person.setId (originalId);

	person.lastName=ui.lastNameInput->text ().simplified ();
	person.firstName=ui.firstNameInput->text ().simplified ();
	person.club=ui.clubInput->currentText ().simplified ();
	person.comments=ui.commentsInput->text ().simplified ();

	// Error checks

	if (isNone (person.lastName))
		errorCheck ("Es wurde kein Nachname angegeben.",
			ui.lastNameInput);

	if (isNone (person.firstName))
		errorCheck ("Es wurde kein Vorname angegeben.",
			ui.firstNameInput);


	return person;
}
