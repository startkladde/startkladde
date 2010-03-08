#include "FlightWindow.h"

#include <cassert>
#include <iostream>

#include <QCompleter>
#include <QShowEvent>

#include "src/color.h"
#include "src/text.h"
#include "src/gui/dialogs.h"
#include "src/gui/windows/EntitySelectWindow.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/db/cache/Cache.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/config/Options.h" // Required for opts.ort
#include "src/util/qString.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/db/DbManager.h"

/*
 * On enabling/diabling widgets:
 *   - The labels are hidden using SkLabel::setConcealed rather than
 *     QLabel::setVisible in order not to modify the layout.
 *   - For each widget, one or more (for pilots) labels are associated via a
 *     QMultiMap.
 *   - We use the widget as indication of which widget to show/hide rather than
 *     the label (which would make sense for hiding people's first and last
 *     name simultaneously) because otherwise we would have to map to both
 *     QWidgets and SkLabels (needed explicitly because of setConcealed).
 *   - The labels are smaller than the widgets which means they have to be
 *     artificially made as big as the widgets so the layout doesn't change
 *     when the widgets are hidden and the labels concealed. This is done in
 *     showEvent. We cannot set the layout's row heights because we cannot find
 *     out which row a given widget is in.
 *   - For the time inputs, both the checkBox and the timeEdit are associated
 *     with the label. This is because the label has to be the minimum size of
 *     both widgets. The timeEdit will be shown/hidden on checkBox value
 *     changes using the visible property, so this won't affect the label.
 *   - If there should be a problem with
 *       (a) widgets from multiple lines being associated with the same label
 *           (name fields), or
 *       (b) the time inputs being shown/hidden with the label and checkbox
 *           visible,
 *     the widgetLabelMap can be split into a widgetLabelMap and a
 *     labelWidgetMap.
 */

/*
 * More random notes:
 *   - there used to be a flight buffer which was updated on every value change
 *     in order to perform continuous error checking. However, this is a state
 *     duplication which significantly complicates the code while providing only
 *     a moderate performance improvement. Thus, it was removed. For error
 *     checking, the values are always read from the input widgets.
 */

using namespace std;

// ***************
// ** Constants **
// ***************

static const QColor errorColor (255, 127, 127);

// ************************
// ** Construction/Setup **
// ************************

/**
 * Creates the flight editor window.
 *
 * setup must be called after cr
 *
 * @param parent the parent widget of the editor window. Passed to the base
 *               class constructor.
 * @param mode the editor mode. This determines among other things, how flight
 *             is treated.
 * @param manager the database manager to use for reading and writing data
 * @param flight the flight to edit or to display in the editing fields
 *               initially, or NULL for none. The flight is copied by the
 *               constructor and not accessed any more later.
 * @param flags the window flags, passed to the base class constructor
 */
FlightWindow::FlightWindow (QWidget *parent, FlightWindow::Mode mode, DbManager &manager, Qt::WindowFlags flags)
	:QDialog (parent, flags),
	manager (manager),
	cache (manager.getCache ()),
	mode (mode),
	labelHeightsSet (false),
	originalFlightId (invalidId),
	selectedPlane (invalidId),
	selectedTowplane (invalidId),
	selectedPilot (invalidId),
	selectedCopilot (invalidId),
	selectedTowpilot (invalidId)
{
	// *** Setup the window
	ui.setupUi (this);


	// *** Setup the data
	fillData ();


	// *** Setup the label map
	/*
	 * Notes:
	 *   - the *pilotLabels are assigned to the corresponding last name widget.
	 *     This is because the are in the same line and the information is also
	 *     used for setting label heights. The first and last name widgets are
	 *     always shown and hidden together anyway.
	 */
	widgetLabelMap.insert (ui.registrationInput, ui.registrationLabel);
	widgetLabelMap.insert (ui.planeTypeLabel   , ui.planeTypeLabel);
	widgetLabelMap.insert (ui.flightTypeInput  , ui.flightTypeLabel);
	//
	widgetLabelMap.insert (ui.pilotLastNameInput   , ui.pilotLastNameLabel);
	widgetLabelMap.insert (ui.pilotFirstNameInput  , ui.pilotFirstNameLabel);
	widgetLabelMap.insert (ui.pilotLastNameInput   , ui.pilotLabel);
	widgetLabelMap.insert (ui.copilotLastNameInput , ui.copilotLastNameLabel);
	widgetLabelMap.insert (ui.copilotFirstNameInput, ui.copilotFirstNameLabel);
	widgetLabelMap.insert (ui.copilotLastNameInput , ui.copilotLabel);
	//
	widgetLabelMap.insert (ui.flightModeInput  , ui.flightModeLabel);
	widgetLabelMap.insert (ui.launchMethodInput, ui.launchMethodLabel);
	//
	widgetLabelMap.insert (ui.towplaneRegistrationInput, ui.towplaneRegistrationLabel);
	widgetLabelMap.insert (ui.towplaneTypeWidget       , ui.towplaneTypeLabel);
	widgetLabelMap.insert (ui.towpilotLastNameInput    , ui.towpilotLabel);
	widgetLabelMap.insert (ui.towpilotLastNameInput    , ui.towpilotLastNameLabel);
	widgetLabelMap.insert (ui.towpilotFirstNameInput   , ui.towpilotFirstNameLabel);
	widgetLabelMap.insert (ui.towflightModeInput       , ui.towflightModeLabel);
	//
	widgetLabelMap.insert (ui.launchTimeCheckbox, ui.launchTimeLabel);
	widgetLabelMap.insert (ui.launchTimeInput, ui.launchTimeLabel);
	widgetLabelMap.insert (ui.landingTimeCheckbox, ui.landingTimeLabel);
	widgetLabelMap.insert (ui.landingTimeInput, ui.landingTimeLabel);
	widgetLabelMap.insert (ui.towflightLandingTimeCheckbox, ui.towflightLandingTimeLabel);
	widgetLabelMap.insert (ui.towflightLandingTimeInput, ui.towflightLandingTimeLabel);
	//
	widgetLabelMap.insert (ui.departureLocationInput       , ui.departureLocationLabel);
	widgetLabelMap.insert (ui.landingLocationInput         , ui.landingLocationLabel);
	widgetLabelMap.insert (ui.towflightLandingLocationInput, ui.towflightLandingLocationLabel);
	widgetLabelMap.insert (ui.numLandingsInput             , ui.numLandingsLabel);
	//
	widgetLabelMap.insert (ui.commentInput       , ui.commentLabel);
	widgetLabelMap.insert (ui.accountingNoteInput, ui.accountingNoteLabel);
	widgetLabelMap.insert (ui.dateInput          , ui.dateLabel);
	widgetLabelMap.insert (ui.errorList          , ui.errorLabel);

	// *** Setup the GUI
	switch (mode)
	{
		case modeCreate:
			setCaption ("Flug anlegen");
			break;
		case modeEdit:
			setCaption ("Flug editieren");
			break;
	}

	// Setup the label error colors
	foreach (SkLabel * const &label, widgetLabelMap.values ())
		label->setErrorColor (errorColor);

	// Setup the required field label colors
	const QColor requiredFieldColor=interpol (0.75, backgroundColor(), Qt::white);
	ui.registrationLabel         ->setDefaultBackgroundColor (requiredFieldColor);
	ui.pilotLabel                ->setDefaultBackgroundColor (requiredFieldColor);
	ui.copilotLabel              ->setDefaultBackgroundColor (requiredFieldColor);
	ui.towpilotLabel             ->setDefaultBackgroundColor (requiredFieldColor);
	ui.flightTypeLabel           ->setDefaultBackgroundColor (requiredFieldColor);
	ui.flightModeLabel           ->setDefaultBackgroundColor (requiredFieldColor);
	ui.launchMethodLabel         ->setDefaultBackgroundColor (requiredFieldColor);
	ui.towplaneRegistrationLabel ->setDefaultBackgroundColor (requiredFieldColor);

	// Setup initial values
	if (mode==modeCreate)
		ui.departureLocationInput->setCurrentText (opts.ort);

	updateSetup ();
	updateErrors (false);

	ui.registrationInput->setFocus ();
}

FlightWindow::~FlightWindow ()
{
}


// ***********
// ** Setup **
// ***********

void FlightWindow::fillData ()
{
	// *** Plane registrations
	const QStringList registrations=cache.getPlaneRegistrations ();

	ui.registrationInput->addItems (registrations);
	ui.registrationInput->setCurrentText (Plane::defaultRegistrationPrefix ());
	ui.registrationInput->setDefaultPrefix (Plane::defaultRegistrationPrefix ());

	ui.towplaneRegistrationInput->addItems (registrations);
	ui.towplaneRegistrationInput->setCurrentText (Plane::defaultRegistrationPrefix ());
	ui.towplaneRegistrationInput->setDefaultPrefix (Plane::defaultRegistrationPrefix ());


	// *** Flight types
	const QList<Flight::Type> flightTypes=Flight::listTypes (false);
	for (int i=0; i<flightTypes.size(); ++i)
		ui.flightTypeInput->addItem (Flight::typeText (flightTypes.at (i), true), flightTypes.at (i));


	// *** Person names
	const QStringList firstNames=cache.getPersonFirstNames();
	const QStringList lastNames =cache.getPersonLastNames ();

	ui.   pilotFirstNameInput ->addItems (firstNames);
	ui. copilotFirstNameInput ->addItems (firstNames);
	ui.towpilotFirstNameInput ->addItems (firstNames);

	ui.   pilotLastNameInput ->addItems (lastNames);
	ui. copilotLastNameInput ->addItems (lastNames);
	ui.towpilotLastNameInput ->addItems (lastNames);

	ui.   pilotFirstNameInput ->setCurrentText("");
	ui. copilotFirstNameInput ->setCurrentText("");
	ui.towpilotFirstNameInput ->setCurrentText("");

	ui.   pilotLastNameInput ->setCurrentText("");
	ui. copilotLastNameInput ->setCurrentText("");
	ui.towpilotLastNameInput ->setCurrentText("");


	// *** Flight flightModes
	const QList<Flight::Mode> flightModes=Flight::listModes (false);
	for (int i=0; i<flightModes.size (); ++i)
		ui.flightModeInput->addItem (Flight::modeText (flightModes.at (i)), flightModes.at (i));


	// *** Towflight flightModes
	const QList<Flight::Mode> towflightModes=Flight::listTowModes (false);
	for (int i=0; i<towflightModes.size(); ++i)
		ui.towflightModeInput->addItem (Flight::modeText (towflightModes.at (i)), towflightModes.at (i));


	// *** Launch methods
	QList<LaunchMethod> launchMethods=cache.getLaunchMethods ().getList ();
	ui.launchMethodInput->addItem ("-", invalidId);
	for (int i=0; i<launchMethods.size (); ++i)
		ui.launchMethodInput->addItem (launchMethods.at (i).nameWithShortcut (), launchMethods.at (i).getId ());


	// *** Locations
	const QStringList locations=cache.getLocations ();
	ui.       departureLocationInput -> insertItem ("");
	ui.         landingLocationInput -> insertItem ("");
	ui.towflightLandingLocationInput -> insertItem ("");

	ui.       departureLocationInput -> insertStringList (locations);
	ui.         landingLocationInput -> insertStringList (locations);
	ui.towflightLandingLocationInput -> insertStringList (locations);

	// Make sure opts.ort is in the list
	ui.       departureLocationInput ->setCurrentText (opts.ort);
	ui.         landingLocationInput ->setCurrentText (opts.ort);
	ui.towflightLandingLocationInput ->setCurrentText (opts.ort);

	ui.         landingLocationInput ->setCurrentText ("");
	ui.towflightLandingLocationInput ->setCurrentText ("");


	// *** Accounting notes
	QStringList abhins;
	ui.accountingNoteInput->addItems (cache.getAccountingNotes ());
	ui.accountingNoteInput->setCurrentText ("");
}

void FlightWindow::showEvent (QShowEvent *event)
{
	QDialog::showEvent (event);

	// We dont't set the label heights on spantaneous show events (generated
	// by the window system, e. g. after the window has been minimized).
	if (!event->spontaneous ())
	{
		if (!labelHeightsSet)
		{
			// Set the heights of the labels to the heights of the widgets so the
			// layout won't changed when the widgets are hidden (the labels are not
			// hidden but SkLabel::concealed).

			// For the time editor, the height returned by sizeHint seems to be 1
			// pixel too low (the layout moves by 1 pixel when a timeEdit is
			// hidden). Therefore, we make the labels (layout rows) one pixel
			// higher than the size hint. To compensate the change in layout, we
			// decrease the verticalSpacing by one, unless it is already 0.
			QGridLayout *layout=dynamic_cast<QGridLayout *> (ui.inputFieldPane->layout ());
			int verticalSpacing=layout->verticalSpacing ();
			if (verticalSpacing>0)
				layout->setVerticalSpacing (verticalSpacing-1);

			QMapIterator<QWidget *, SkLabel *> i (widgetLabelMap);
			while (i.hasNext ())
			{
				i.next ();

				QWidget *widget=i.key ();
				SkLabel *label=i.value ();

				// There may be multiple widgets associated with one label, for
				// example for the time fields.
				label->setMinimumHeight (1+qMax (widget->sizeHint ().height (), label->sizeHint ().height ()));
			}

			labelHeightsSet=true;
		}
	}
}


// ****************
// ** Invocation **
// ****************

/*
 * Invocation notes:
 *   For modeless windows, the invocation methods will return immediately.
 *   This has several implications:
 *     - The window has to be created on the heap. To make sure it is deleted,
 *       its DeleteOnClose attribute is set.
 *     - If the flight is accepted by the user, it is written to the database
 *       by the accepting slot.
 */

void FlightWindow::createFlight (QWidget *parent, DbManager &manager, QDate date)
{
	FlightWindow *w=new FlightWindow (parent, modeCreate, manager, NULL);
	w->setAttribute (Qt::WA_DeleteOnClose, true);

	w->ui.dateInput->setDate (date);
	w->updateSetup ();

	w->exec ();
}

void FlightWindow::repeatFlight (QWidget *parent, DbManager &manager, const Flight &original, QDate date)
{
	FlightWindow *w=new FlightWindow (parent, modeCreate, manager);
	w->setAttribute (Qt::WA_DeleteOnClose, true);
	w->flightToFields (original, true);

	w->ui.dateInput->setDate (date);
	w->updateSetup ();

	w->ui.launchMethodInput->setFocus ();

	w->exec ();
}

void FlightWindow::editFlight (QWidget *parent, DbManager &manager, Flight &flight)
{
	FlightWindow *w=new FlightWindow (parent, modeEdit, manager);
	w->setAttribute (Qt::WA_DeleteOnClose, true);
	w->flightToFields (flight, false);

	w->updateSetup ();
	w->updateErrors (true);

	w->exec ();
}



// **********************
// ** Input field data **
// **********************

/**
 * Write name parts (either first names or last names) to a list box - either
 * only those which match another name part (for example, only the first names
 * matching a given last name) or all values for a name part, if the other name
 * part is empty.
 *
 * The name parts are read from the Db::Cache::Cache.
 *
 * Additionally, if the name part is unique, it is written to the target list
 * box's current text if that was empty before. This can be prevented using the
 * preserveTarget parameter.
 *
 * @param fullListMethod the method of Db::Cache::Cache used to retrieve all values
 *                       of the name part
 * @param partialListMethod the method of Db::Cache::Cache used to retrieve all
 *                          values of the name part which match a given other
 *                          name part
 * @param target the combo box to write the name parts to
 * @param otherName the given other name part
 * @param preserveTarget whether to preserve the current text of the target
 *                       combo box, even if it is empty
 * @return the number of matching names (not: people)
 */
int FlightWindow::fillNames (QStringList (Db::Cache::Cache::*fullListMethod)(), QStringList (Db::Cache::Cache::*partialListMethod)(const QString &), QComboBox *target, const QString &otherName, bool preserveTarget)
{
	// Store the old value of the target field
	QString oldValue=target->currentText ();

	// Get the list of name parts from the data storage, either the full list
	// or only the ones that match the other name part given.
	QStringList nameList;
	if (otherName.simplified ().isEmpty ())
		nameList=(cache.*fullListMethod)();
	else
		nameList=(cache.*partialListMethod)(otherName);

	// Write the name list to the target
	target->clear ();
	target->addItems (nameList);

	// If there is exactly one name and the target field was empty before and
	// doesn't have to be preserved, and the source name is not empty, write
	// the unique name to the target field.
	if (oldValue.isEmpty () && !preserveTarget && nameList.size ()==1 && !otherName.simplified ().isEmpty ())
		target->setCurrentText (nameList.at (0));
	else
		target->setCurrentText (oldValue);

	return nameList.size ();
}

dbId FlightWindow::fillFirstNames  (bool active, QComboBox *target, const QString &lastName, bool preserveTarget)
{
	if (!active) return invalidId;

	fillNames (
		&Db::Cache::Cache::getPersonFirstNames,
		&Db::Cache::Cache::getPersonFirstNames,
		target, lastName, preserveTarget);

	// Even if there were multiple matching other name parts, the current
	// combination may still be unique. If it is, return the person's ID.
	return cache.getUniquePersonIdByName (target->currentText (), lastName);
}

dbId FlightWindow::fillLastNames  (bool active, QComboBox *target, const QString &firstName, bool preserveTarget)
{
	if (!active) return invalidId;

	fillNames (
		&Db::Cache::Cache::getPersonLastNames,
		&Db::Cache::Cache::getPersonLastNames,
		target, firstName, preserveTarget);

	// Even if there were multiple matching other name parts, the current
	// combination may still be unique. If it is, return the person's ID.
	return cache.getUniquePersonIdByName (firstName, target->currentText ());
}

// ******************
// ** Error checks **
// ******************

void FlightWindow::updateErrors (bool setFocus)
{
	if (!isErrorListActive ()) return;

	Flight flight=determineFlightBasic ();

	Plane *plane              =cache.getNewObject<Plane       > (flight.planeId        );
	Plane *towplane           =cache.getNewObject<Plane       > (flight.towplaneId     );
	LaunchMethod *launchMethod=cache.getNewObject<LaunchMethod> (flight.launchMethodId );

	FlightError error;
	int errorIndex=0;
	int numErrors=0;
	QWidget *firstErrorWidget=NULL;

	// Reset the error state of all labels
	foreach (SkLabel * const & label, widgetLabelMap.values ())
		label->setError (false);

	ui.errorList->clear ();
	while ((error=flight.errorCheck (&errorIndex, true, launchMethod && launchMethod->isAirtow (), plane, towplane, launchMethod))!=ff_ok)
	{
		// In the cases of unknown or non-unique people, we don't want to query
		// the user. So the determineFlightBasic method uses the buffered IDs
		// (selectedX, see header), which may be invalid if the corresponding
		// entity is unknown or non-unique. This may lead to erroneous "not
		// specified" errors being reported. Check the input fields if
		// something is specified and skip the error if necessary.

		bool skipError=false;

		bool    planeSpecified=!(eintrag_ist_leer (getCurrentRegistration         ()) || getCurrentRegistration         ()==Plane::defaultRegistrationPrefix ());
		bool towplaneSpecified=!(eintrag_ist_leer (getCurrentTowplaneRegistration ()) || getCurrentTowplaneRegistration ()==Plane::defaultRegistrationPrefix ());
		bool    pilotSpecified=!eintraege_sind_leer (getCurrentPilotLastName    (), getCurrentPilotFirstName    ());
		bool  copilotSpecified=!eintraege_sind_leer (getCurrentCopilotLastName  (), getCurrentCopilotFirstName  ());
		//bool towpilotSpecified=!eintraege_sind_leer (getCurrentTowpilotLastName (), getCurrentTowpilotFirstName ());

		// Potential non-error: something has been specified but is unknown or
		// non-unique, so the ID is invalid.
		if (error==ff_kein_flugzeug           &&    planeSpecified) skipError=true;
		if (error==ff_kein_pilot              &&    pilotSpecified) skipError=true;
		if (error==ff_schulung_ohne_begleiter &&  copilotSpecified) skipError=true;
		if (error==ff_kein_schleppflugzeug    && towplaneSpecified) skipError=true;

		if (!skipError)
		{
			++numErrors;

			ui.errorList->addItem (flight.errorDescription (error));

			QWidget *errorWidget=getErrorWidget (error);
			if (errorWidget)
			{
				if (!firstErrorWidget) firstErrorWidget=errorWidget;

				// There may be multiple labels for the widget. This uses the most
				// recently inserted one.
				SkLabel *errorWidgetLabel=widgetLabelMap.value (errorWidget);
				if (errorWidgetLabel)
					errorWidgetLabel->setError (true);
			}
		}
	}

	delete plane;
	delete towplane;
	delete launchMethod;

	if (setFocus && firstErrorWidget)
		firstErrorWidget->setFocus ();

	if (numErrors==0)
	{
		ui.errorList->addItem ("Keine");
		ui.errorList->setPalette (palette ());
	}
	else
	{
		QPalette pal=ui.errorList->palette ();
		pal.setColor (QPalette::Base, errorColor);
		ui.errorList->setPalette (pal);
	}
}

// Fehlerbehandlung
QWidget *FlightWindow::getErrorWidget (FlightError error)
{
	switch (error)
	{
		case ff_keine_id:                             return NULL;
		case ff_ok:                                   return NULL;
		case ff_kein_flugzeug:                        return ui.registrationInput;
		case ff_kein_pilot:                           return ui.pilotLastNameInput;
		case ff_pilot_gleich_begleiter:               return ui.copilotLastNameInput;
		case ff_pilot_nur_vorname:                    return ui.pilotLastNameInput;
		case ff_pilot_nur_nachname:                   return ui.pilotFirstNameInput;
		case ff_pilot_nicht_identifiziert:            return ui.pilotLastNameInput;
		case ff_begleiter_nur_vorname:                return ui.copilotLastNameInput;
		case ff_begleiter_nur_nachname:               return ui.copilotFirstNameInput;
		case ff_begleiter_nicht_identifiziert:        return ui.copilotLastNameInput;
		case ff_schulung_ohne_begleiter:              return ui.copilotLastNameInput;
		case ff_begleiter_nicht_erlaubt:              return ui.copilotLastNameInput;
		case ff_nur_gelandet:                         return ui.landingTimeInput;
		case ff_landung_vor_start:                    return ui.landingTimeInput;
		case ff_keine_startart:                       return ui.launchMethodInput;
		case ff_kein_modus:                           return ui.flightModeInput;
		case ff_kein_sfz_modus:                       return ui.towflightModeInput;
		case ff_kein_flugtyp:                         return ui.flightTypeInput;
		case ff_landungen_negativ:                    return ui.numLandingsInput;
		case ff_doppelsitzige_schulung_in_einsitzer:  return ui.copilotLastNameInput;
		case ff_kein_startort:                        return ui.departureLocationInput;
		case ff_kein_zielort:                         return ui.landingLocationInput;
		case ff_kein_zielort_sfz:                     return ui.towflightLandingLocationInput;
		case ff_segelflugzeug_landungen:              return ui.numLandingsInput;
		case ff_begleiter_in_einsitzer:               return ui.copilotLastNameInput;
		case ff_gastflug_in_einsitzer:                return ui.flightTypeInput;
		case ff_segelflugzeug_selbststart:            return ui.launchMethodInput;
		case ff_schlepp_nur_gelandet:                 return ui.towflightLandingTimeInput;
		case ff_schlepp_landung_vor_start:            return ui.towflightLandingTimeInput;
		case ff_landungen_null:                       return ui.numLandingsInput;
		case ff_landungen_ohne_start:                 return ui.numLandingsInput;
		case ff_segelflugzeug_landungen_ohne_landung: return ui.numLandingsInput;
		case ff_startort_gleich_zielort:              return ui.landingLocationInput;
		case ff_kein_schleppflugzeug:                 return ui.towplaneRegistrationInput;
		case ff_towplane_is_glider:                   return ui.towplaneRegistrationInput;
		case ff_pilot_gleich_towpilot:                return ui.pilotLastNameInput;
		case ff_towpilot_nur_vorname:                 return ui.towpilotLastNameInput;
		case ff_towpilot_nur_nachname:                return ui.towpilotFirstNameInput;
		case ff_towpilot_nicht_identifiziert:         return ui.towpilotLastNameInput;
		// No default to allow compiler warnings
	}

	return NULL;
}




// ****************************
// ** Flight reading/writing **
// ****************************

void FlightWindow::personToFields (dbId id, SkComboBox *lastNameInput, SkComboBox *firstNameInput, QString incompleteLastName, QString incompleteFirstName)
{
	// Note that filling the name parts is done here rather than from
	// updateSetup because that function is called on every field change.
	bool ok=false;

	if (idValid (id))
	{
		try
		{
			Person person=cache.getObject<Person> (id);
			 lastNameInput->setCurrentText (person.lastName);
			firstNameInput->setCurrentText (person.firstName );
			ok=true;
		}
		catch (Db::Cache::Cache::NotFoundException &ex) {}
	}

	if (!ok)
	{
		 lastNameInput->setCurrentText (incompleteLastName );
		firstNameInput->setCurrentText (incompleteFirstName);
	}

	fillFirstNames (true, firstNameInput, lastNameInput->currentText  (), true);
	fillLastNames  (true, lastNameInput , firstNameInput->currentText (), true);
}

void FlightWindow::planeToFields (dbId id, SkComboBox *registrationInput, SkLabel *typeLabel)
{
	// Note that filling the plane type is done here rather than from
	// updateSetup because that function is called on every field change.
	if (idValid (id))
	{
		try
		{
			Plane plane=cache.getObject<Plane> (id);
			registrationInput->setCurrentText (plane.registration);
			typeLabel->setText (plane.type);
		}
		catch (Db::Cache::Cache::NotFoundException &ex) {}
	}
}

void FlightWindow::flightToFields (const Flight &flight, bool repeat)
{
	// Note that for repeating, some fields are not set or set differently.

	originalFlightId = flight.getId ();
	selectedPlane    = flight.planeId;
	selectedTowplane = flight.towplaneId;
	selectedPilot    = flight.pilotId;
	selectedCopilot  = flight.copilotId;
	selectedTowpilot = flight.towpilotId;

	planeToFields (flight.planeId, ui.registrationInput, ui.planeTypeWidget);
	ui.flightTypeInput->setCurrentItemByItemData (flight.type);

	// space

	personToFields (flight.pilotId    , ui.pilotLastNameInput  , ui.pilotFirstNameInput  , flight.pilotLastName, flight.pilotFirstName);
	personToFields (flight.copilotId, ui.copilotLastNameInput, ui.copilotFirstNameInput, flight.copilotLastName, flight.copilotFirstName);

	// space

	ui.flightModeInput->setCurrentItemByItemData (flight.mode);

	// Launch method: on repeating a flight, the launch method is not copied because
	// it may be different from before (different winch). An exception is self
	// launch becaus it is unlikely that a plane which did a self launch will
	// use another type of launch later.
	bool copyLaunchMethod=!repeat;

	try
	{
		if (idValid (flight.launchMethodId))
			if (cache.getObject<LaunchMethod> (flight.launchMethodId).type==LaunchMethod::typeSelf)
				copyLaunchMethod=true;
	}
	catch (Db::Cache::Cache::NotFoundException &ex)
	{
		log_error ("Launch method not found in FlightWindow::flightToFields");
	}

	if (copyLaunchMethod) ui.launchMethodInput->setCurrentItemByItemData (flight.launchMethodId);
	launchMethodChanged (ui.launchMethodInput->currentIndex ());

	// The towplane is set even if it's not an airtow in case the user selects
	// an unknown airtow launchMethod later.
	planeToFields (flight.towplaneId, ui.towplaneRegistrationInput, ui.towplaneTypeWidget);
	personToFields (flight.towpilotId, ui.towpilotLastNameInput, ui.towpilotFirstNameInput, flight.towpilotLastName, flight.towpilotFirstName);
	ui.towflightModeInput->setCurrentItemByItemData (flight.towflightMode);

	// space

	if (!repeat)
	{
		ui.          launchTimeCheckbox->setChecked (getTimeFieldCheckboxValue (flight.departed   ));
		ui.         landingTimeCheckbox->setChecked (getTimeFieldCheckboxValue (flight.landed    ));
		ui.towflightLandingTimeCheckbox->setChecked (getTimeFieldCheckboxValue (flight.towflightLanded));

		ui.          launchTimeInput->setTime (flight.departureTime                .get_qtime (tz_utc)); // Even if not active
		ui.         landingTimeInput->setTime (flight.landingTime                .get_qtime (tz_utc)); // Even if not active
		ui.towflightLandingTimeInput->setTime (flight.towflightLandingTime.get_qtime (tz_utc)); // Even if not active
	}

	// space

	ui.departureLocationInput->setCurrentText (flight.departureLocation);
	ui.landingLocationInput->setCurrentText (flight.landingLocation);
	ui.towflightLandingLocationInput->setCurrentText (flight.landingLocation);
	if (!repeat) ui.numLandingsInput->setValue (flight.numLandings);

	// space

	if (!repeat) ui.commentInput->setText (flight.comments);
	ui.accountingNoteInput->setCurrentText (flight.accountingNotes);
	ui.dateInput->setDate (flight.getEffectiveDate (tz_utc, QDate::currentDate ()));

#undef PLANE
#undef PERSON
}

Flight FlightWindow::determineFlightBasic () throw ()
{
	Flight flight;

	// Some of the data is taken from the stored data
	flight.setId (originalFlightId);
	flight.planeId     = isRegistrationActive         ()?selectedPlane   :invalidId;
	flight.towplaneId  = isTowplaneRegistrationActive ()?selectedTowplane:invalidId;
	flight.pilotId     = isPilotActive                ()?selectedPilot   :invalidId;
	flight.copilotId   = isCopilotActive              ()?selectedCopilot :invalidId;
	flight.towpilotId  = isTowpilotActive             ()?selectedTowpilot:invalidId;


	// Some of the data can just be copied to the flight.
	// Registration: may have to query user
	if (isFlightTypeActive                   ()) flight.type            =getCurrentFlightType ();
	//
	// Pilot: may have to query user
	// Copilot: may have to query user
	//
	if (isFlightModeActive                   ()) flight.mode              =getCurrentFlightMode ();
	if (isLaunchMethodActive                 ()) flight.launchMethodId      =getCurrentLaunchMethodId ();
	//
	// Towplane registration: may have to query user
	// Towpilot: may have to query user
	if (isTowflightModeActive                ()) flight.towflightMode          =getCurrentTowflightMode ();
	//
	if (isLaunchActive                       ()) flight.departed          =isLaunchTimeActive ();
	if (isLandingActive                      ()) flight.landed           =isLandingTimeActive ();
	if (isTowflightLandingActive             ()) flight.towflightLanded       =isTowflightLandingTimeActive ();
	// Launch time: set with date
	// Landing time: set with date
	// Towflight landing time: set with date
	//
	if (isDepartureLocationActive        ()) flight.departureLocation        =getCurrentDepartureLocation ();
	if (isLandingLocationActive          ()) flight.landingLocation          =getCurrentLandingLocation ();
	if (isTowflightLandingLocationActive ()) flight.towflightLandingLocation =getCurrentTowflightLandingLocation ();
	if (isNumLandingsActive              ()) flight.numLandings              =getCurrentNumLandings ();
	//
	if (isCommentActive                      ()) flight.comments        =getCurrentComment ();
	if (isAccountingNodeActive               ()) flight.accountingNotes =getCurrentAccountingNote ();
	// getCurrentDate


	// Setting the times requires combining the date and time fields
	QDate date= (isDateActive()) ? (getCurrentDate ()) : QDate::currentDate ();
#define SET_TIME(active, target, value) do { if (active) target.set_to (date, value, /*tz_utc, */true); else target.set_null (); } while (0)
	SET_TIME (isLaunchTimeActive           (), flight.departureTime,                 getCurrentLaunchTime           ());
	SET_TIME (isLandingTimeActive          (), flight.landingTime,                 getCurrentLandingTime          ());
	SET_TIME (isTowflightLandingTimeActive (), flight.towflightLandingTime, getCurrentTowflightLandingTime ());
#undef SET_TIME


	return flight;
}

void FlightWindow::errorCheck (const QString &problem, QWidget *widget)
	throw (FlightWindow::AbortedException)
{
	if (!confirmProblem (this, "Fehler", problem))
	{
		if (widget) widget->setFocus ();
		throw AbortedException ();
	}
}

void FlightWindow::checkFlightPhase1 (const Flight &flight, bool launchNow)
	throw (FlightWindow::AbortedException)
{
	// Phase 1: plane determined, towplane and people not determined

	// Note that we use the values from the passed flight, not from the editor
	// fields.

	if ((launchNow || flight.departed) && flight.departsHere () && idInvalid (flight.launchMethodId))
		errorCheck ("Es wurde keine Startartart angegeben.",
			ui.launchMethodInput);

	if ((flight.departed || !flight.departsHere ()) && eintrag_ist_leer (flight.departureLocation))
		errorCheck ("Es wurde kein Startort angegeben.",
				ui.departureLocationInput);

	if ((flight.landed || !flight.landsHere ()) && eintrag_ist_leer (flight.landingLocation))
		errorCheck ("Es wurde kein Zielort angegeben.",
			ui.landingLocationInput);

	if ((flight.departsHere ()!=flight.landsHere ()) && (flight.departureLocation.simplified ()==flight.landingLocation.simplified ()))
		errorCheck ("Der Startort ist gleich dem Zielort.",
			flight.departsHere ()?ui.landingLocationInput:ui.departureLocationInput);

	if (flight.landed && flight.departed && flight.departureTime>flight.landingTime)
		errorCheck ("Die Landezeit des Flugs liegt vor der Startzeit.",
			ui.landingTimeInput);

	if (flight.departsHere () && flight.landsHere () && flight.landed && !flight.departed)
		errorCheck ("Es wurde eine Landezeit, aber keine Startzeit angegeben.",
				ui.landingTimeInput);

	if (flight.landsHere () && flight.landed && flight.numLandings==0)
		errorCheck ("Es wurde eine Landezeit angegeben, aber die Anzahl der Landungen ist 0.",
			ui.numLandingsInput);

	if (flight.towflightLanded && !flight.towflightLandsHere () && eintrag_ist_leer (flight.towflightLandingLocation))
		errorCheck (utf8 ("Es wurde kein Zielort für das Schleppflugzeug angegeben."),
			ui.towflightLandingLocationInput);

	if (flight.departed && flight.towflightLanded && flight.departureTime>flight.towflightLandingTime)
		errorCheck ("Die Landezeit des Schleppflugs liegt vor der Startzeit.",
			ui.towflightLandingTimeInput);

	if (flight.towflightLanded && !flight.departed)
		errorCheck ("Es wurde eine Landezeit des Schleppflugs, aber keine Startzeit angegeben.",
			ui.towflightLandingTimeInput);
}

void FlightWindow::checkFlightPhase2 (const Flight &flight, bool launchNow, const Plane *plane, const Plane *towplane, const LaunchMethod *launchMethod)
	throw (FlightWindow::AbortedException)
{
	// Phase 2: plane and towplane determined, people not determined

	if (idValid (flight.planeId) && flight.planeId==flight.towplaneId)
		errorCheck ("Flugzeug und Schleppflugzeug sind identisch.",
			ui.towplaneRegistrationInput);

	if (plane && launchMethod &&
		flight.numLandings>1 && plane->category==Plane::categoryGlider && !(launchMethod && launchMethod->type==LaunchMethod::typeAirtow))
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) ein Segelflugzeug.\nEs wurde jedoch mehr als eine Landung angegeben.")
			.arg (plane->registration).arg (plane->type),
			ui.numLandingsInput);

	if (plane &&
		flight.numLandings>0 && !flight.landed && plane->category==Plane::categoryGlider && !(launchMethod && launchMethod->type==LaunchMethod::typeAirtow))
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) ein Segelflugzeug.\nEs wurden jedoch eine Landung, aber keine Landezeit angegeben.")
			.arg (plane->registration).arg (plane->type),
			ui.numLandingsInput);

	if (plane &&
		plane->category==Plane::categoryGlider && launchMethod && launchMethod->type==LaunchMethod::typeSelf)
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) ein Segelflugzeug.\nEs wurden jedoch \"Selbststart\" als Startart angegeben.")
			.arg (plane->registration).arg (plane->type),
			ui.launchMethodInput);

	if (plane && launchMethod &&
		(plane->category==Plane::categoryAirplane || plane->category==Plane::categoryUltralight) && launchMethod->type!=LaunchMethod::typeSelf)
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) ein Motorflugzeug.\nEs wurden jedoch eine andere Startart als \"Selbststart\" angegeben.")
			.arg (plane->registration).arg (plane->type),
			ui.launchTimeInput);

	if (plane && launchMethod &&
		plane->numSeats==1 && (flight.type==Flight::typeGuestPrivate || flight.type==Flight::typeGuestExternal) && launchMethod->type!=LaunchMethod::typeSelf)
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) einsitzig.\nEs wurden jedoch der Flugtyp \"Gastflug\" angegeben.")
			.arg (plane->registration).arg (plane->type),
			ui.registrationInput);

	if (plane && launchMethod &&
		plane->numSeats==1 && flight.type==Flight::typeTraining2 && launchMethod->type!=LaunchMethod::typeSelf)
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) einsitzig.\nEs wurden jedoch der Flugtyp \"Doppelsitzige Schulung\" angegeben.")
			.arg (plane->registration).arg (plane->type),
			ui.registrationInput);

	if (towplane &&
		towplane->category==Plane::categoryGlider)
		errorCheck (QString ("Laut Datenbank ist das Schleppflugzeug %1 (%2) ein Segelflugzeug.")
			.arg (plane->registration).arg (plane->type),
			ui.towplaneRegistrationInput);

	if (plane && launchNow && idValid (cache.planeFlying (plane->getId ())))
		errorCheck (QString ("Laut Datenbank fliegt das Flugzeug %1 noch.")
			.arg (plane->registration),
			ui.registrationInput);

	if (towplane && launchNow && idValid (cache.planeFlying (towplane->getId ())))
		errorCheck (QString ("Laut Datenbank fliegt das Schleppflugzeug %1 noch.")
			.arg (towplane->registration),
			ui.registrationInput);
}

void FlightWindow::checkFlightPhase3 (const Flight &flight, bool launchNow, const Plane *plane, const Person *pilot, const Person *copilot, const Person *towpilot)
	throw (FlightWindow::AbortedException)
{
	// Phase 3: plane, towplane and people determined

	// Pilot und Begleiter identisch

	if (idValid (flight.pilotId) && flight.pilotId==flight.copilotId)
		errorCheck ("Pilot und Begleiter sind identisch.",
			ui.pilotLastNameInput);

	if (idValid (flight.pilotId) && flight.pilotId==flight.towpilotId)
		errorCheck ("Pilot und Schlepppilot sind identisch.",
			ui.towpilotLastNameInput);

	if (idValid (flight.copilotId) && flight.copilotId==flight.towpilotId)
		errorCheck ("Begleiter und Schlepppilot sind identisch.",
			ui.towpilotLastNameInput);

	if (flight.type==Flight::typeTraining2 && !flight.copilotSpecified ())
		errorCheck ("Doppelsitzige Schulung ohne Fluglehrer.",
			ui.copilotLastNameInput);

	if (plane && plane->numSeats==1 && flight.copilotSpecified ())
		errorCheck (QString ("Laut Datenbank ist das Flugzeug %1 (%2) einsitzig.\nEs wurden jedoch ein Begleiter angegeben.")
		.arg (plane->registration).arg (plane->type),
		ui.registrationInput);

	if (pilot && launchNow && idValid (cache.personFlying (pilot->getId ())))
		errorCheck (QString ("Laut Datenbank fliegt der Pilot %1 %2 noch.")
			.arg (pilot->firstName).arg (pilot->lastName),
			ui.pilotLastNameInput);

	if (copilot && launchNow && idValid (cache.personFlying (copilot->getId ())))
		errorCheck (QString ("Laut Datenbank fliegt der Begleiter %1 %2 noch.")
			.arg (copilot->firstName).arg (copilot->lastName),
			ui.copilotLastNameInput);

	if (towpilot && launchNow && idValid (cache.personFlying (towpilot->getId ())))
		errorCheck (QString ("Laut Datenbank fliegt der Schlepppilot %1 %2 noch.")
			.arg (towpilot->firstName).arg (towpilot->lastName),
			ui.towpilotLastNameInput);
}

void FlightWindow::determineFlightPlanes (Flight &flight)
	throw (FlightWindow::AbortedException)
{
	// Determine the plane
	if (isRegistrationActive ())
		flight.planeId=determinePlane (getCurrentRegistration (), "Flugzeug", ui.registrationInput);

	// Determine the towplane
	// TODO: for a known airtow launch method, we should give the user the
	// possibility to enter the plane
	if (isTowplaneRegistrationActive ())
		flight.towplaneId=determinePlane (getCurrentTowplaneRegistration (), "Schleppflugzeug", ui.towplaneRegistrationInput);
}

void FlightWindow::determineFlightPeople (Flight &flight, const LaunchMethod *launchMethod)
	throw (FlightWindow::AbortedException)
{
	bool pilotRequired=true;
	if (!flight.departsHere ()) pilotRequired=false;
	if (launchMethod && launchMethod->type!=LaunchMethod::typeSelf) pilotRequired=false;

	// Determine the pilot
	selectedPilot=
	flight.pilotId=
		determinePerson (
			isPilotActive(),
			getCurrentPilotFirstName (),
			getCurrentPilotLastName (),
			Flight::typePilotDescription (getCurrentFlightType ()),
			pilotRequired,
			flight.pilotFirstName, flight.pilotLastName,
			selectedPilot,
			ui.pilotLastNameInput);

	// Determine the copilot
	selectedCopilot=
	flight.copilotId=
		determinePerson (
			isCopilotActive (),
			getCurrentCopilotFirstName (),
			getCurrentCopilotLastName (),
			Flight::typeCopilotDescription (getCurrentFlightType ()),
			false, // Copilot is never required; flight instructor is checked later
			flight.copilotFirstName, flight.copilotLastName,
			selectedCopilot,
			ui.copilotLastNameInput);

	// Determine the towpilot
	selectedTowpilot=
	flight.towpilotId=
		determinePerson (
			isTowpilotActive(),
			getCurrentTowpilotFirstName(),
			getCurrentTowpilotLastName(),
			"Schlepppilot",
			true, // required
			flight.towpilotFirstName, flight.towpilotLastName,
			selectedTowpilot,
			ui.towpilotLastNameInput);
}

/**
 * Reads a flight from the input fields, querying the user for additional input
 * (unknown planes, multiple people) if necessary. Additional data is read from
 * the cache as required.
 *
 * Also does some error checks and asks the user if he wants to accept anyway.
 *
 * @return the flight, if there are no errors in the flight or all errors have
 *         been confirmed by the user
 * @throw AbortedException if the user aborts on data input or warning
 */
Flight FlightWindow::determineFlight (bool launchNow)
	throw (FlightWindow::AbortedException)
{
	/*
	 * Notes:
	 *   - for some of the data, we have to query the user. This may result
	 *     in an AbortedException
	 *   - people are determined last because if the person's name is
	 *     non-unique, the user has to be queried and we don't want to do that
	 *     more often than necessary
	 */

	Plane *plane=NULL, *towplane=NULL;
	Person *pilot=NULL, *copilot=NULL, *towpilot=NULL;
	LaunchMethod *launchMethod=NULL;

	try
	{
		// Phase 1: basic data
		Flight flight=determineFlightBasic ();

		launchMethod=cache.getNewObject<LaunchMethod> (flight.launchMethodId);

		checkFlightPhase1 (flight, launchNow);

		// Phase 2: planes
		determineFlightPlanes (flight);

		plane   =cache.getNewObject<Plane> (flight.planeId);
		towplane=cache.getNewObject<Plane> (flight.towplaneId);

		checkFlightPhase2 (flight, launchNow, plane, towplane, launchMethod);

		// Phase 3: people
		determineFlightPeople (flight, launchMethod);

		pilot   =cache.getNewObject<Person> (flight.pilotId    );
		copilot =cache.getNewObject<Person> (flight.copilotId);
		towpilot=cache.getNewObject<Person> (flight.towpilotId );

		checkFlightPhase3 (flight, launchNow, plane, pilot, copilot, towpilot);

		return flight;
	}
	catch (...)
	{
		delete plane;
		delete towplane;

		delete pilot;
		delete copilot;
		delete towpilot;

		delete launchMethod;

		throw;
	}
}


/**
 * Tries to determine the plane for a given registration by retrieving
 * additional data from the dataSource and querying the user if necessary.
 *
 * This method takes the following steps, in order, and uses the first result:
 *  - If no registration is given, the user is asked if this is on purpose
 *  - If there is a plane with the specified registration, it is used
 *  - If there is a plane with the specified registration and the default
 *    registration prefix prepended, it is used
 *  - The user is asked if he wants to addObject the plane to the database
 * If none of these succeeds, an AbortedException is thrown
 *
 * @param registration the registration given by the user
 * @param description the description of the plane for displaying to the user.
 *                    This can be used to distinguish between differen planes.
 * @return the ID of the resulting plane, or invalid if "no plane" was
 *         confirmed by the user
 * @throw AbortedException if the user aborted the selection
 */
dbId FlightWindow::determinePlane (QString registration, QString description, QWidget *widget)
	throw (FlightWindow::AbortedException)
{
	dbId id=invalidId;

	// Check if no registration is given. Return true if the user confirms or
	// false else.
	if (eintrag_ist_leer (registration) || registration.simplified ().toLower()==Plane::defaultRegistrationPrefix ().simplified ().toLower ())
	{
		// No registration given. Query the user to accept this.
		if (!confirmProblem (this,
			QString ("Kein %1 angegeben").arg (description),
			QString ("Es wurde kein %1 angegeben.").arg (description)))
			throw AbortedException ();

		// User accepted
		return invalidId;
	}

	// Try to get the ID for the plane with the given registration. Return if
	// found.
	id=cache.getPlaneIdByRegistration (registration);
	if (!idInvalid (id))
		return id;

	// Try to get the ID for the plane with the given registration with the
	// registration prefix prepended. Return if found and the user confirms it.
	id=cache.getPlaneIdByRegistration (Plane::defaultRegistrationPrefix ()+registration);
	if (idValid (id))
	{
		QString title=QString ("%1 nicht bekannt").arg (description);
		QString question=QString (
			"Das %1 %2 ist nicht bekannt.\n"
			"Es gibt allerdings ein Flugzeug mit dem Kennzeichen %3.\n"
			"Soll dieses Flugzeug verwendet werden?")
			.arg (description, registration, Plane::defaultRegistrationPrefix ()+registration);

		if (yesNoQuestion (this, title, question))
			return id;
	}

	QString title=QString ("%1 nicht bekannt").arg (description);
	QString question=QString (
		"Das %1 %2 ist nicht bekannt.\n"
		"Soll es in die Datenbank aufgenommen werden?")
		.arg (description, registration);

	if (yesNoQuestion (this, title, question))
	{
		dbId result=ObjectEditorWindow<Plane>::createObject (this, manager);
		if (idValid (result))
			return result;
		else
		{
			widget->setFocus ();
			throw AbortedException ();
		}
	}
	else
		throw AbortedException ();

	throw AbortedException ();
}

dbId FlightWindow::createNewPerson (QString lastName, QString firstName)
	throw (AbortedException)
{
	Person person;
	person.firstName=firstName;
	person.lastName=lastName;

	dbId result=ObjectEditorWindow<Person>::createObject (this, manager);
	if (idValid (result))
		return result;
	else
		throw AbortedException ();
}

/**
 * Tries to determine the person for a given first name and last name by
 * retrieving additional data from the dataSource and querying the user if
 * necessary.
 *
 * This method takes the following steps:
 *  - If no name is given, the user is asked if this is on purpose (except if
 *    required is false, in which case invalid is returned)
 *  - If a name is given, but it does not exist, ask the user to addObject the person
 *    to the database
 *  - If a name is given and there is a unique person of this name in the
 *    database, the person is returned
 *  - In any other case, the user is shown a list of potential persons and an
 *    "Unknown" and a "Create new" button.
 * If none of these succeeds, an AbortedException is thrown
 *
 * @param active whether the person is active at all; an invalid ID is returned
 *               if the person is not used
 * @param firstName the first name of the person given by the user
 * @param lastName the last name of the person given by the user
 * @param description the description of the person for displaying to the user.
 *                    This can be used to distinguish between differen people.
 * @param required whether the person is required for a regular flight. If a
 *                 person (for example, the pilot) is required, the user has
 *                 to confirm if no name was given.
 * @return the ID of the resulting person, or invalid if "unknown" was
 *         confirmed by the user
 * @throw AbortedException if the user aborted the selection
 */
dbId FlightWindow::determinePerson (bool active, QString firstName, QString lastName, QString description, bool required, QString &incompleteFirstName, QString &incompleteLastName, dbId originalId, QWidget *widget)
	throw (FlightWindow::AbortedException)
{
	if (!active) return invalidId;

	/*
	 *  This is what can happen here:
	 *
	 *  # | Name given | Req'd | Candidates | Action
	 *  --+------------+-------+------------+----------------------------------
	 *  0 | is '+1'    | X     | 0          | Confirm: go on or AbortedException
	 *  1 | Complete   | X     | 0          | Confirm: Add or AbortedException
	 *  2 | Complete   | X     | 1          | Return name
	 *  3 | Complete   | X     | >=1        | Selection list ("Multiple candidates")
	 *  4 | Part       | X     | 0          | Selection list ("Only partial name given")
	 *  5 | Part       | X     | 1          | Selection list ("Only partial name given")
	 *  6 | Part       | X     | >=1        | Selection list ("Only partial name given")
	 *  7 | None       | Yes   | N/A        | Confirm: None or AbortedException
	 *  8 | None       | No    | N/A        | Invalid ID
	 *
	 *  Result from the selection list:
	 *    - Ok: return that person
	 *    - Create new: create a new person
	 *      - Ok: addObject person to database and return it
	 *      - Cancelled: AbortedException
	 *    - Unknown: Store incomplete name part(s)
	 *    - Canceled/no selection: AbortedException
	 *
	 * Note that for non-unique persons, it is OK to store both incomplete name
	 * parts, but unknown persons are required to be added to the database.
	 * This is a pragmatic approach and there may be better solutions.
	 */

	bool firstNameGiven=!eintrag_ist_leer (firstName);
	bool lastNameGiven=!eintrag_ist_leer (lastName);

	// Case 0: name is "+1"
	if (firstName.simplified ()=="+1" || lastName.simplified ()=="+1")
	{
		QString title=utf8 ("+1 als Name angegeben");
		QString problem=utf8 ("Es wurde \"+1\" als Name angegeben. Für Gastflüge sollte"
			" stattdessen der Flugtyp \"Gastflug\" verwendet werden.");
		if (!confirmProblem (this, title, problem))
			throw AbortedException ();
	}

	// Case 7 and 8: no name was given
	if (!firstNameGiven && !lastNameGiven)
	{
		// Case 8: no name required
		if (!required)
			return invalidId;

		// Case 7: confirm that the name is not known
		QString title=QString ("Kein %1 angegeben").arg (description);
		QString problem=QString ("Es wurde kein %1 angegeben.").arg (description);

		if (!confirmProblem (this, title, problem))
		{
			widget->setFocus ();
			throw AbortedException ();
		}
		else
			return invalidId;
	}

	// Get a list of candidates, using all name information available.
	QList<dbId> candidates;
	if (firstNameGiven && lastNameGiven)
		candidates=cache.getPersonIdsByName (firstName, lastName);
	else if (firstNameGiven)
		candidates=cache.getPersonIdsByFirstName (firstName);
	else if (lastNameGiven)
		candidates=cache.getPersonIdsByLastName (lastName);

	// Case 1: complete name given, but no person found
	if (firstNameGiven && lastNameGiven && candidates.empty ())
	{
		// No person of that name was found in the database.
		QString title=QString ("%1 nicht bekannt").arg (description);
		QString question=QString (
			"Die Person %1 %2 (%3) ist nicht bekannt.\n"
			"Soll sie in die Datenbank aufgenommen werden?")
			.arg (firstName, lastName, description);

		if (yesNoQuestion (this, title, question))
			return createNewPerson (lastName, firstName);
		else
			throw AbortedException ();
	}

	// Case 2: complete name given and uniqe person found
	if (firstNameGiven && lastNameGiven && candidates.size ()==1)
		return candidates.at (0);

	// Case 3-6: show selection list with candidates, "Unknown" and "Create"
	// options and cancel button

	// There were multiple persons. Let the user select one.
	EntitySelectWindow<Person> selector (this, "selector");

	QString title ("Personenauswahl");
	QString text;
	if (firstNameGiven && lastNameGiven)
		// Case 3: multiple candidates
		text=utf8 ("Es kommen mehrere Personen in Frage. Bitte auswählen (%1):").arg (description);
	else if (!firstNameGiven)
		// Case 4-6: no first name given
		text=utf8 ("Es wurde nur ein Nachname angegeben. Bitte auswählen (%1):").arg (description);
	else if (!lastNameGiven)
		// Case 4-6: no last name given
		text=utf8 ("Es wurde nur ein Vorname angegeben. Bitte auswählen (%1):").arg (description);
	// Note that (!firstNameGiven && !lastNameGiven) has already been handled
	// (case 1)

	// Get all matching people (candidates) from the database
	QList<Person> people=cache.getPeople (candidates);

	// Determine the preselected person
	dbId preselectionId=0;
	if (idValid (originalId))
		preselectionId=originalId;

	// Do the selection
	selection_result res=selector.do_selection (title, text, people, preselectionId);

	switch (res)
	{
		case sr_ok:
			return selector.get_result_id ();
		case sr_unknown:
			// Unknown person
			incompleteLastName=lastName;
			incompleteFirstName=firstName;
			return 0;
		case sr_new:
			// Create new
			return createNewPerson (lastName, firstName);
		case sr_cancelled: case sr_none_selected:
			throw AbortedException ();
	}

	log_error ("Unhandled case in FlightWindow::determinePerson");
	return 0;
}


// **************
// ** Database **
// **************

bool FlightWindow::writeToDatabase (Flight &flight)
{
	bool success=false;

	// FIXME handle OperationCanceledException

	switch (mode)
	{
		// TODO background
		case modeCreate:
		{
			try
			{
				success=manager.createObject (flight, this);
			}
			catch (OperationCanceledException)
			{
				// TODO the cache may now be inconsistent
			}
		} break;
		case modeEdit:
		{
			try
			{
				// May return 0 if nothing changed
				manager.updateObject (flight, this);
				success=true;
			}
			catch (OperationCanceledException)
			{
				// TODO the cache may now be inconsistent
			}
		} break;
	}

	// The task completed. But was it successful?
	if (!success)
	{
		QMessageBox::message (
			"Fehler beim Speichern",
			QString ("Fehler beim Speichern"),
			"&OK",
			this
			);
	}

	return success;
}


// *****************************
// ** Input field active-ness **
// *****************************

bool FlightWindow::currentIsAirtow ()
{
	if (!isLaunchMethodActive ()) return false;
	if (idInvalid (getCurrentLaunchMethodId ())) return false;

	try
	{
		return getCurrentLaunchMethod ().isAirtow ();
	}
	catch (Db::Cache::Cache::NotFoundException &ex)
	{
		return false;
	}
}

bool FlightWindow::isTowplaneRegistrationActive ()
{
	if (!currentIsAirtow ()) return false;
	if (!idValid (getCurrentLaunchMethodId ())) return false;

	try
	{
		return !getCurrentLaunchMethod ().towplaneKnown ();
	}
	catch (Db::Cache::Cache::NotFoundException &ex)
	{
		return false;
	}
}

// ***************************
// ** Input value frontends **
// ***************************

LaunchMethod FlightWindow::getCurrentLaunchMethod ()
{
	dbId id=getCurrentLaunchMethodId ();
	return cache.getObject<LaunchMethod> (id);
}


// *******************************
// ** Input field state helpers **
// *******************************


bool FlightWindow::isNowActionPossible ()
{
	// As can be seen from the table, the "Now" action is only possible under
	// the following conditions:
	//   - create mode
	//   - none of the times specified
	//   - the date is today

	if (mode!=modeCreate) return false;

	if (isLaunchTimeActive ()) return false;
	if (isLandingTimeActive ()) return false;
	if (isTowflightLandingTimeActive ()) return false;

	if (getCurrentDate ()!=QDate::currentDate ()) return false;

	// If none of the tests, failed, the "Now" action is possible.
	return true;
}

void FlightWindow::enableWidget (QWidget *widget, bool enabled)
{
	widget->setVisible (enabled);

	QList<SkLabel *> values=widgetLabelMap.values (widget);
	for (int i=0; i<values.size(); ++i)
		// Note the use of setConcealed instead of setVisible/setHidden
		values.at (i)->setConcealed (!enabled);
}

void FlightWindow::disableWidget (QWidget *widget, bool disabled)
{
	enableWidget (widget, !disabled);
}

void FlightWindow::enableWidgets (QWidget *widget0, QWidget *widget1, bool enabled)
{
	enableWidget (widget0, enabled);
	enableWidget (widget1, enabled);
}

void FlightWindow::disableWidgets (QWidget *widget0, QWidget *widget2, bool disabled)
{
	disableWidget (widget0, disabled);
	disableWidget (widget2, disabled);
}

// ******************************
// ** Input field state update **
// ******************************

// TODO rename to departureTime
const QString textLaunchTimeAutomatic           =utf8 ("Automati&sch");
const QString textLaunchTimeLaunched            =utf8 ("Ge&startet"  );
const QString textLandingTimeAutomatic          =utf8 ("&Automatisch");
const QString textLandingTimeLanded             =utf8 ("Gel&andet"   );
const QString textTowflightLandingTimeAutomatic =utf8 ("Au&tomatisch");
const QString textTowflightLandingTimeLanded    =utf8 ("Gelande&t"   );
const QString textTowflightLandingTimeEnded     =utf8 ("Beende&t"    );
const QString textTowflightLandingTime          =utf8 ("Landezeit Schl&eppflugzeug:");
const QString textTowflightEnd                  =utf8 ("Schl&eppende:");
const QString textButtonOk                      =utf8 ("&OK");
const QString textButtonLaunchNow               =utf8 ( "&Jetzt starten");
const QString textButtonLaunchLater             =utf8 ("S&päter starten");
const QString textButtonLandNow                 =utf8 ( "&Jetzt landen");
const QString textButtonLandLater               =utf8 ("S&päter landen");


/*
 * Notes on field state updates:
 *   - Changes in some field values (e. g. flight type) lead to changes in some
 *     fields' visbility or labels (e. g. copilot's name).
 *   - Field visibility should not depend on data read from the database because
 *     that may be unreliable. Also, we only use the data from the input fields
 *     (getCurrent... methods), never from the flight buffer.
 *   - The matching name parts and plane types are not part of the state.
 *   - The field states are only updated if the value change was caused by user
 *     interaction, not by a programmatic change. This is done to avoid
 *     infinite loops of fields affecting each other (e. g. name pairs) and
 *     changes on intermediate value changes.
 *   - We need the functionality to setup all field states, for example after
 *     loading a flight.
 *   - To avoid code duplication, the state updating functionality is not in the
 *     handlers for the change event. Also, some changes may cause indirect
 *     changes. So all field states are updated on each change of a field value
 *     that can cause any state change.
 *   - This could be improved, for example by using flags for which states need
 *     to be updated, but some multiple states can depend on the same field and
 *     some states depend on multiple fields, so this can get quite complex. It
 *     is probably not worth the work.
 *   - State updates are grouped by widget, not by cause, because some widgets
 *     are affected by multiple causes (e. g. tow flight landing time: by
 *     flightMode.startsHere and launchMethod.isAirtow)
 */

void FlightWindow::updateSetupVisibility ()
{
	//registrationInput - always visible
	//ui.planeTypeWidget - always visible
	//flightTypeInput - always visible
	//
	//pilotLastNameInput, pilotFirstNameInput - always visible
	enableWidgets (ui.copilotLastNameInput, ui.copilotFirstNameInput   , isCopilotActive                      ());
	//
	//flightModeInput - always visible
	enableWidget  (ui.launchMethodInput                                , isLaunchMethodActive                 ());
	//
	enableWidget  (ui.towplaneRegistrationInput                        , isTowplaneRegistrationActive         ());
	enableWidget  (ui.towplaneTypeWidget                               , isTowplaneTypeActive                 ());
	enableWidgets (ui.towpilotFirstNameInput, ui.towpilotLastNameInput , isTowpilotActive                     ());
	enableWidget  (ui.towflightModeInput                               , isTowflightModeActive                ());
	//
	enableWidget (ui.launchTimeCheckbox                                , isLaunchActive                       ());
	ui.launchTimeInput->setVisible                                      (isLaunchTimeActive                   ());
	enableWidget (ui.landingTimeCheckbox                               , isLandingActive                      ());
	ui.landingTimeInput->setVisible                                     (isLandingTimeActive                  ());
	enableWidget (ui.towflightLandingTimeCheckbox                      , isTowflightLandingActive             ());
	ui.towflightLandingTimeInput->setVisible                            (isTowflightLandingTimeActive         ());
	//
	//departureLocationInput - always visible
	//landingLocationInput - always visible
	enableWidget (ui.towflightLandingLocationInput,                      isTowflightLandingLocationActive ());
	//numLandingsInput - always visible
	//
	//commentInput - always visible
	//accountingNoteInput - always visible
	//dateInput - always visible
	enableWidget (ui.errorList,                                          isErrorListActive                    ());
}

void FlightWindow::updateSetupLabels ()
{
	switch (mode)
	{
		case modeCreate:
			// In create mode, we have "Automatic" checkboxes
			ui.          launchTimeCheckbox->setText (textLaunchTimeAutomatic);
			ui.         landingTimeCheckbox->setText (textLandingTimeAutomatic);
			ui.towflightLandingTimeCheckbox->setText (textTowflightLandingTimeAutomatic);
			break;
		case modeEdit:
			// In edit mode, we have "Launched"/"Landed" checkboxes
			ui.          launchTimeCheckbox->setText (textLaunchTimeLaunched);
			ui.         landingTimeCheckbox->setText (textLandingTimeLanded);
			ui.towflightLandingTimeCheckbox->setText (currentTowLandsHere ()?textTowflightLandingTimeLanded:textTowflightLandingTimeEnded);
			break;
	}

	ui.towflightLandingTimeLabel->setText (currentTowLandsHere ()?textTowflightLandingTime:textTowflightEnd);

	ui.pilotLabel  ->setText (Flight::typePilotDescription   (getCurrentFlightType ())+":");
	ui.copilotLabel->setText (Flight::typeCopilotDescription (getCurrentFlightType ())+":");
}

void FlightWindow::updateSetupButtons ()
{
	bool nowPossible=isNowActionPossible ();
	if (nowPossible && currentStartsHere ())
	{
		// Launch now/later
		ui.nowButton->setVisible(true);
		ui.nowButton->setText (textButtonLaunchNow);
		ui.okButton->setText (textButtonLaunchLater);
	}
	else if (nowPossible && currentLandsHere ())
	{
		// Land now/later
		ui.nowButton->setVisible(true);
		ui.nowButton->setText (textButtonLandNow);
		ui.okButton->setText (textButtonLandLater);
	}
	else
	{
		ui.nowButton->setVisible (false);
		ui.okButton->setText (textButtonOk);
	}
}

void FlightWindow::updateSetup ()
{
	updateSetupVisibility ();
	updateSetupLabels ();
	updateSetupButtons ();
}



// *******************************
// ** Input field value helpers **
// *******************************



// *************************************
// ** Input field value change events **
// *************************************

/*
 * Notes about these functions:
 *   - Change other fields as appropriate and call the xChanged method.
 *   - Don't call updateSetup or updateErrors - this is done by the caller.
 */

void FlightWindow::registrationChanged (const QString &text)
{
	// Find out the plane ID
	dbId id=cache.getPlaneIdByRegistration (text);
	selectedPlane=id;

	if (idValid (id))
	{
		// Get the plane
		try
		{
			Plane plane=cache.getObject<Plane> (id);

			// Set the plane type widget
			ui.planeTypeWidget->setText (plane.type);

			// For planes that only do self launches, set the launch method to "self
			// launch" if it is not currently set to anything else.
			if (plane.selfLaunchOnly () && idInvalid (getCurrentLaunchMethodId ()))
				ui.launchMethodInput->setCurrentItemByItemData (cache.getLaunchMethodByType (LaunchMethod::typeSelf));
		}
		catch (Db::Cache::Cache::NotFoundException &ex)
		{
			ui.planeTypeWidget->setText ("?");
		}
	}
	else
	{
		ui.planeTypeWidget->setText ("-");
	}
}


void FlightWindow::flightModeChanged (int index)
{
	Flight::Mode flightMode=(Flight::Mode)ui.flightModeInput->itemData (index).toInt ();

	if (mode==modeCreate)
	{
		const QString departureLocation=ui.departureLocationInput->currentText ();
		const QString   landingLocation=ui.  landingLocationInput->currentText ();

		if (Flight::departsHere (flightMode))
		{
			// Departure location is local location
			if (locationEntryCanBeChanged (departureLocation))
				ui.departureLocationInput->setCurrentText (opts.ort);

			// Clear landing location (leaving or set automatically on landing)
			if (locationEntryCanBeChanged (landingLocation))
				ui.landingLocationInput->setCurrentText ("");
		}
		else
		{
			// Clear departure location (not departed here)
			if (locationEntryCanBeChanged (departureLocation))
				ui.departureLocationInput->setCurrentText ("");

			// Landing location will be set automatically on landing
		}
	}
}

void FlightWindow::launchMethodChanged (int index)
{
	dbId launchMethodId=(dbId)ui.launchMethodInput->itemData (index).toLongLong ();;

	if (idValid (launchMethodId))
	{
		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (launchMethodId);

		if (launchMethod.isAirtow ())
		{
			QString towplaneRegistration=launchMethod.towplaneKnown () ? launchMethod.towplaneRegistration : getCurrentTowplaneRegistration ();
			dbId towplaneId=cache.getPlaneIdByRegistration (towplaneRegistration);
			if (idValid (towplaneId))
			{
				try
				{
					Plane towplane=cache.getObject<Plane> (towplaneId);
					ui.towplaneTypeWidget->setText (towplane.type);
				}
				catch (Db::Cache::Cache::NotFoundException &ex)
				{
					ui.towplaneTypeWidget->setText ("?");
				}
			}

		}
	}
}

// Space

void FlightWindow::towplaneRegistrationChanged (const QString &text)
{
	// Find out the plane ID
	dbId id=cache.getPlaneIdByRegistration (text);
	selectedTowplane=id;

	if (idValid (id))
	{
		try
		{
			// Get the plane and set the type widget
			Plane towplane=cache.getObject<Plane> (id);

			// Set the plane type widget
			ui.towplaneTypeWidget->setText (towplane.type);
		}
		catch (Db::Cache::Cache::NotFoundException &ex)
		{
			ui.towplaneTypeWidget->setText ("?");
		}
	}
	else
	{
		ui.planeTypeWidget->setText ("-");
	}
}

void FlightWindow::landingTimeCheckboxChanged (bool checked)
{
	bool landed=getTimeFieldActive (checked);

	if (landed)
	{
		// Landed => set landing location to local location
		if (locationEntryCanBeChanged (ui.landingLocationInput->currentText ()))
			ui.landingLocationInput->setCurrentText (opts.ort);

		// Set 1 landing if it was 0.
		if (getCurrentNumLandings ()==0)
			ui.numLandingsInput->setValue (1);
	}
	else
	{
		// Not landed => unset landing location input
		if (locationEntryCanBeChanged (ui.landingLocationInput->currentText ()))
			ui.landingLocationInput->setCurrentText ("");
	}
}


void FlightWindow::towflightLandingTimeCheckboxChanged (bool checked)
{
	bool towflightLanded=getTimeFieldActive (checked);

	if (currentTowLandsHere() && towflightLanded)
	{
		// Landed => set landing location to local location
		if (locationEntryCanBeChanged (ui.towflightLandingLocationInput->currentText ()))
			ui.towflightLandingLocationInput->setCurrentText (opts.ort);
	}
	else
	{
		// Not landed => unset landing location input
		if (locationEntryCanBeChanged (ui.towflightLandingLocationInput->currentText ()))
			ui.towflightLandingLocationInput->setCurrentText ("");
	}
}




// *******************
// ** Button events **
// *******************

void FlightWindow::on_okButton_clicked()
{
	// The "launch later"/"land later"/"ok" button was pressed. Check and store
	// the flight without launching it.

	try
	{
		Flight flight=determineFlight (false);
		if (writeToDatabase (flight))
			accept (); // Close the dialog
	}
	catch (AbortedException &e)
	{
		// User aborted, do nothing
	}
}

void FlightWindow::on_nowButton_clicked ()
{
	try
	{
		Flight flight=determineFlight (true);

		// If we are not in create mode, the date is not today or the auto
		// fields are not checked, the button is not visible at all.
		if (currentStartsHere ())
		{
			flight.departed=true;
			flight.departureTime.set_current (true);
		}
		else
		{
			flight.landed=true;
			flight.landingTime.set_current (true);
		}

		if (writeToDatabase (flight))
			accept (); // Close the dialog
	}
	catch (AbortedException &e)
	{
		// User aborted, do nothing
	}
}
