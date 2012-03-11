#include "PersonEditorPane.h"

#include "src/db/cache/Cache.h"
#include "src/model/Person.h"
#include "src/text.h"
#include "src/config/Settings.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"

/*
 * TODO: disallow person name changes; allow merges only
 */

// ******************
// ** Construction **
// ******************

PersonEditorPane::PersonEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent):
	ObjectEditorPane<Person> (mode, cache, parent)
{
	ui.setupUi (this);

	// Not pre-set because this prevents us from making the dialog smaller in
	// Designer
	ui.medicalCheckDisabledLabel->setText (tr ("Medical check is disabled!"));
	ui.medicalCheckDisabledLabel->setVisible (false);

	fillData ();

	ui.lastNameInput->setFocus ();
}

PersonEditorPane::~PersonEditorPane ()
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

	ui.checkMedicalInput->addItem (tr ("Yes"), true );
	ui.checkMedicalInput->addItem (tr ("No" ), false);
	ui.checkMedicalInput->setCurrentItemByItemData (false);
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


// ****************
// ** GUI events **
// ****************

void PersonEditorPane::on_medicalValidityUnknownCheckbox_toggled ()
{
	ui.medicalValidityInput->setEnabled (!ui.medicalValidityUnknownCheckbox->isChecked ());
}

void PersonEditorPane::on_checkMedicalInput_currentIndexChanged ()
{
	ui.medicalCheckDisabledLabel->setVisible (
		ui.checkMedicalInput->currentItemData ().toBool ()==true &&
		!Settings::instance ().checkMedicals
	);
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PersonEditorPane::objectToFields (const Person &person)
{
	ui.lastNameInput->setText (person.lastName);
	ui.firstNameInput->setText (person.firstName);
	ui.clubInput->setEditText (person.club);
	ui.commentsInput->setText (person.comments);
	ui.checkMedicalInput->setCurrentItemByItemData (person.checkMedical);
	if (person.medicalValidity.isValid ())
	{
		ui.medicalValidityInput->setDate (person.medicalValidity);
		ui.medicalValidityUnknownCheckbox->setChecked (false);
	}
	else
	{
		ui.medicalValidityInput->setDate (QDate::currentDate ());
		ui.medicalValidityUnknownCheckbox->setChecked (true);
	}
	ui.clubIdInput->setText (person.clubId);
}

QDate PersonEditorPane::getEffectiveMedicalValidity ()
{
	if (ui.medicalValidityUnknownCheckbox->isChecked ())
		return QDate ();
	else
		return ui.medicalValidityInput->date ();
}

void PersonEditorPane::fieldsToObject (Person &person)
{
	person.lastName             =ui.lastNameInput       ->text ().simplified ();
	person.firstName            =ui.firstNameInput      ->text ().simplified ();
	person.club                 =ui.clubInput           ->currentText ().simplified ();
	person.comments             =ui.commentsInput       ->text ().simplified ();
	person.checkMedical         =ui.checkMedicalInput   ->currentItemData ().toBool ();
	person.medicalValidity      =getEffectiveMedicalValidity ();
	person.clubId               =ui.clubIdInput         ->text ();

	// Error checks

	if (isNone (person.lastName))
		errorCheck (tr ("Last name not specified."),
			ui.lastNameInput);

	if (isNone (person.firstName))
		errorCheck (tr ("First name not specified."),
			ui.firstNameInput);
}
