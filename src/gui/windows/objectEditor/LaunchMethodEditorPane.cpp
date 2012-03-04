#include "LaunchMethodEditorPane.h"

#include "src/model/LaunchMethod.h"
#include "src/db/cache/Cache.h"
#include "src/util/qString.h"
#include "src/notr.h"

// ******************
// ** Construction **
// ******************

LaunchMethodEditorPane::LaunchMethodEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent):
	ObjectEditorPane<LaunchMethod> (mode, cache, parent)
{
	ui.setupUi(this);

	fillData ();

	ui.nameInput->setFocus ();
}

LaunchMethodEditorPane::~LaunchMethodEditorPane()
{

}

template<> ObjectEditorPane<LaunchMethod> *ObjectEditorPane<LaunchMethod>::create (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent)
{
	return new LaunchMethodEditorPane (mode, cache, parent);
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
	ui.personRequiredInput->addItem (tr ("Yes"), true );
	ui.personRequiredInput->addItem (tr ("No") , false);
	ui.personRequiredInput->setCurrentItemByItemData (true);

	// Registrations
	ui.towplaneRegistrationInput->addItem ("");
	ui.towplaneRegistrationInput->addItems (cache.getPlaneRegistrations ());
	ui.towplaneRegistrationInput->setEditText ("");
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
	ui.nameInput                ->setText                  (launchMethod.name);
	ui.shortNameInput           ->setText                  (launchMethod.shortName);
	ui.logStringInput           ->setText                  (launchMethod.logString);
	ui.keyboardShortcutInput    ->setText                  (launchMethod.keyboardShortcut);
	ui.typeInput                ->setCurrentItemByItemData (launchMethod.type);
	ui.towplaneRegistrationInput->setEditText              (launchMethod.towplaneRegistration);
	ui.personRequiredInput      ->setCurrentItemByItemData (launchMethod.personRequired);
	ui.commentsInput            ->setText                  (launchMethod.comments);

	on_typeInput_activated (ui.typeInput->currentIndex ());
}

void LaunchMethodEditorPane::fieldsToObject (LaunchMethod &launchMethod)
{
	launchMethod.name                 = ui.nameInput                     ->text ().simplified ();
	launchMethod.shortName            = ui.shortNameInput                ->text ().simplified ();
	launchMethod.logString            = ui.logStringInput                ->text ().simplified ();
	launchMethod.keyboardShortcut     = ui.keyboardShortcutInput         ->text ().simplified ();
	launchMethod.type                 = (LaunchMethod::Type)ui.typeInput ->currentItemData ().toInt ();
	launchMethod.towplaneRegistration = ui.towplaneRegistrationInput     ->currentText ().simplified ();
	launchMethod.personRequired       = ui.personRequiredInput           ->currentItemData ().toBool ();
	launchMethod.comments             = ui.commentsInput                 ->text ().simplified ();

	// Error checks
	// TODO error checks:
	//   - towplane is glider
	//   - create towplane if it does not exist
	requiredField (launchMethod.name     , ui.nameInput     , tr ("Name not specified"         ));
	requiredField (launchMethod.shortName, ui.shortNameInput, tr ("Short name not specified"   ));
	requiredField (launchMethod.logString, ui.logStringInput, tr ("Logbook label not specified"));
}
