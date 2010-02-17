#include "LaunchMethodEditorPane.h"

#include "src/model/LaunchMethod.h"

#include "src/db/DataStorage.h"


// ******************
// ** Construction **
// ******************

LaunchMethodEditorPane::LaunchMethodEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent):
	ObjectEditorPane<LaunchMethod> (mode, dataStorage, parent)
{
	ui.setupUi(this);

	fillData ();

	ui.nameInput->setFocus ();
}

LaunchMethodEditorPane::~LaunchMethodEditorPane()
{

}

template<> ObjectEditorPane<LaunchMethod> *ObjectEditorPane<LaunchMethod>::create (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent)
{
	return new LaunchMethodEditorPane (mode, dataStorage, parent);
}


// ***********
// ** Setup **
// ***********

void LaunchMethodEditorPane::fillData ()
{
	// Types
	foreach (LaunchMethod::Type type, LaunchMethod::listTypes ())
		ui.typeInput->addItem (LaunchMethod::typeString (type), type);
	ui.typeInput->setCurrentItemByItemData (LaunchMethod::typeOther);

	// Person required
	ui.personRequiredInput->addItem ("Ja"  , true );
	ui.personRequiredInput->addItem ("Nein", false);
	ui.personRequiredInput->setCurrentItemByItemData (true);

	// Registrations
	ui.towplaneRegistrationInput->addItem ("");
	ui.towplaneRegistrationInput->addItems (dataStorage.getPlaneRegistrations ());
	ui.towplaneRegistrationInput->setCurrentText ("");
}

// ****************
// ** GUI events **
// ****************

void LaunchMethodEditorPane::on_typeInput_activated (int index)
{
	bool airtow=(ui.typeInput->itemData (index)==LaunchMethod::typeAirtow);

	ui.towplaneRegistrationInput->setEnabled (airtow);
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void LaunchMethodEditorPane::objectToFields (const LaunchMethod &launchMethod)
{
	originalId=launchMethod.getId ();

	ui.nameInput                ->setText                  (launchMethod.name);
	ui.shortNameInput           ->setText                  (launchMethod.shortName);
	ui.logStringInput           ->setText                  (launchMethod.logString);
	ui.keyboardShortcutInput    ->setText                  (launchMethod.keyboardShortcut);
	ui.typeInput                ->setCurrentItemByItemData (launchMethod.type);
	ui.towplaneRegistrationInput->setCurrentText           (launchMethod.towplaneRegistration);
	ui.personRequiredInput      ->setCurrentItemByItemData (launchMethod.personRequired);
	ui.commentsInput            ->setText                  (launchMethod.comments);

	on_typeInput_activated (ui.typeInput->currentIndex ());
}

LaunchMethod LaunchMethodEditorPane::determineObject ()
{
	LaunchMethod launchMethod (originalId);

	launchMethod.name                 = ui.nameInput                     ->text ();
	launchMethod.shortName            = ui.shortNameInput                ->text ();
	launchMethod.logString            = ui.logStringInput                ->text ();
	launchMethod.keyboardShortcut     = ui.keyboardShortcutInput         ->text ();
	launchMethod.type                 = (LaunchMethod::Type)ui.typeInput ->currentItemData ().toInt ();
	launchMethod.towplaneRegistration = ui.towplaneRegistrationInput     ->currentText ();
	launchMethod.personRequired       = ui.personRequiredInput           ->currentItemData ().toBool ();
	launchMethod.comments             = ui.commentsInput                 ->text ();

	// Error checks
	// TODO error checks:
	//   - towplane is glider
	//   - create towplane if it does not exist
	requiredField (launchMethod.name     , ui.nameInput     , QString::fromUtf8 ("Es wurde kein Name angegeben."                 ));
	requiredField (launchMethod.shortName, ui.shortNameInput, QString::fromUtf8 ("Es wurde kein Kürzel angegeben."               ));
	requiredField (launchMethod.logString, ui.logStringInput, QString::fromUtf8 ("Es wurde keine Flugbuch-Bezeichnung angegeben."));

	return launchMethod;
}
