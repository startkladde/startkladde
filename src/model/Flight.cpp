#include "Flight.h"

#include <iostream>

#include "src/config/Settings.h"
#include "src/db/cache/Cache.h"
#include "src/model/Plane.h"
#include "src/model/LaunchMethod.h"
#include "src/text.h"
#include "src/db/Query.h"
#include "src/db/result/Result.h"
#include "src/util/qString.h"
#include "src/util/time.h"
#include "src/flightColor.h" // TODO remove after flightColor has been moved to Flight

template<class T> class QList;

// TODO Vereinheitlichen der Statusfunktionen untereinander und mit den
// condition-strings
// TODO Errors in other places: for towflights, the landing time is meaningful
// even if !landsHere.
// TODO consider an AbstractFlight and a TowFlightProxy
// TODO: "Currently flying" should be named "still to land" (more flying than
// total flights with prepared incoming flights)
// TODO: should not depend on Cache - move methods to Cache


/*
 * Potential model changes:
 *   - for airtows, always store the towplane ID here and only store a generic
 *     "airtow" launch method
 *   - store the towplane ID or the registration?
 */


// ******************
// ** Construction **
// ******************

Flight::Flight ():
	FlightBase ()
{
	initialize ();
}

Flight::Flight (dbId id):
	FlightBase (id)
{
	initialize ();
}

void Flight::initialize ()
{
	cachedErrorsValid=false;
}


// ****************
// ** Comparison **
// ****************

bool Flight::operator< (const Flight &o) const
{
	return sort (&o)<0;
}

/**
 * Compares the flight with another flight (custom sorting).
 *
 * @return >0 if this flight is later, <0 if this flight is earlier
 */
int Flight::sort (const Flight *other) const
{
	// Both prepared
	if (isPrepared () && other->isPrepared  ())
	{
		// Incoming prepared before locally departing prepared
		if (departsHere () && !other->departsHere ()) return 1;
		if (!departsHere () && other->departsHere ()) return -1;

		// Flights are equal
		return 0;
	}

	// Prepared flights to the end
	if (isPrepared ()) return 1;
	if (other->isPrepared ()) return -1;

	// Sort by effective time
	QDateTime t1=effectiveTime ();
	QDateTime t2=other->effectiveTime ();
	if (t1>t2) return 1;
	if (t1<t2) return -1;
	return 0;
}


// ************
// ** Status **
// ************

int Flight::countFlying (const QList<Flight> flights)
{
	// TODO move to FlightList (to be created)
	int result=0;

	// TODO this is not correct - prepared coming flights should be counted
	// here. Note that this means that flights flying may be greater than
	// flights total - maybe use to "Flugbewegungen"/"Departures/Landings",
	// "planes departed and/or landed" and "planes to land"
	foreach (const Flight &flight, flights)
		if (flight.isFlying ())
			++result;

	return result;
}

int Flight::countHappened (const QList<Flight> flights)
{
	// TODO move to FlightList (to be created)
	int result=0;

	foreach (const Flight &flight, flights)
		if (flight.happened ())
			++result;

	return result;
}

/**
 * Whether the flight either departed or landed; it need not have finished.
 */
// TODO: guarantee that any flight for which this is not true has an effectiveDate
bool Flight::happened () const
{
	if (departsHere () && getDeparted ()) return true;
	if (landsHere () && getLanded ()) return true;
	return false;
}

bool Flight::finished () const
{
	if (isTowflight ())
		// For leaving towflights, landed means ended.
		return getLanded ();
	else
		return (landsHere ()?getLanded ():getDeparted ());
}


// **********
// ** Crew **
// **********

QString Flight::pilotDescription () const
{
	return typePilotDescription (getType ());
}

QString Flight::copilotDescription () const
{
	return typeCopilotDescription (getType ());
}

bool Flight::pilotSpecified () const
{
	return idValid (getPilotId ()) ||
		!isNone (getPilotLastName (), getPilotFirstName ());
}

bool Flight::copilotSpecified () const
{
	return idValid (getCopilotId ()) ||
		!isNone (getCopilotLastName (), getCopilotFirstName ());
}

bool Flight::towpilotSpecified () const
{
	return idValid (getTowpilotId ()) ||
		!isNone (getTowpilotLastName (), getTowpilotFirstName ());
}


QString Flight::incompletePilotName () const
{
	return incompletePersonName (getPilotLastName (), getPilotFirstName ());
}

QString Flight::incompleteCopilotName () const
{
	return incompletePersonName (getCopilotLastName (), getCopilotFirstName ());
}

QString Flight::incompleteTowpilotName () const
	/*
	 * Makes the incomplete name of the towpilot.
	 * Return value:
	 *   - the name.
	 */
{
	return incompletePersonName (getTowpilotLastName (), getTowpilotFirstName ());
}

QString Flight::incompletePersonName (QString nn, QString vn) const
	/*
	 * Makes the incomplete name of a person.
	 * Parameters:
	 *   - nn: the last name.
	 *   - vn: the first name.
	 * Return value:
	 *   - the formatted name.
	 */
{
	if (isNone (nn) && isNone (vn)) return ("-");
	else if (isNone (nn)) return QString ("(???, %1)").arg (vn);
	else if (isNone (vn)) return QString ("(%1, %2)").arg (nn).arg ("???"); // ??) would be a trigraph
	else                            return QString ("%1, %2").arg (nn).arg (vn);
}

// *******************
// ** Launch method **
// *******************

bool Flight::isAirtow (Cache &cache) const
{
	try
	{
		if (idInvalid (getLaunchMethodId ())) return false;

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (getLaunchMethodId ());
		return launchMethod.isAirtow ();
	}
	catch (Cache::NotFoundException)
	{
		return false;
	}
}

dbId Flight::effectiveTowplaneId (Cache &cache) const
{
	try
	{
		if (idInvalid (getLaunchMethodId ())) return invalidId;

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (getLaunchMethodId ());
		if (!launchMethod.isAirtow ()) return invalidId;

		if (launchMethod.towplaneKnown ())
			return cache.getPlaneIdByRegistration (launchMethod.towplaneRegistration);
		else
			return getTowplaneId ();
	}
	catch (Cache::NotFoundException)
	{
		return false;
	}
}




// ***********************
// ** Departure/landing **
// ***********************

#define notPossibleIf(condition, reasonText) do { if (condition) { if (reason) *reason=reasonText; return false; } } while (0)

bool Flight::canDepart (QString *reason) const
{
	// TODO only for flights of today

	// Already landed
	notPossibleIf (landsHere () && getLanded (), "Der Flug ist bereits gelandet");

	// Does not dpeart here
	notPossibleIf (!departsHere (), "Der Flug startet nicht hier.");

	// Already departed
	notPossibleIf (getDeparted (), "Der Flug ist bereits gestartet.");

	return true;
}

bool Flight::canLand (QString *reason) const
{
	// Already landed
	notPossibleIf (getLanded (), "Der Flug ist bereits gelandet.");

	// Does not land here (only applies to non-towflights)
	notPossibleIf (!isTowflight () && !landsHere (), "Der Flug landet nicht hier.");

	// Must depart first
	notPossibleIf (departsHere () && !getDeparted (), "Der Flug ist noch nicht gestartet.");

	return true;
}

bool Flight::canTouchngo (QString *reason) const
{
	// TODO only for flights of today

	// Towflight
	notPossibleIf (isTowflight (), "Der Flug ist ein Schleppflug");

	// Already landed
	notPossibleIf (getLanded (), "Der Flug ist bereits gelandet.");

	// Must depart first
	notPossibleIf (departsHere () && !getDeparted (), "Der Flug ist noch nicht gestartet.");

	return true;
}

bool Flight::canTowflightLand (QString *reason) const
{
	// Already landed
	notPossibleIf (getTowflightLanded (), "Der Schleppflug ist bereits gelandet.");

	// Must depart first
	notPossibleIf (departsHere () && !getDeparted (), "Der Flug ist noch nicht gestartet.");

	return true;
}

#undef notPossibleIf

bool Flight::departNow (bool force)
{
	if (force || canDepart ())
	{
		setDepartureTime (nullSeconds (QDateTime::currentDateTime ().toUTC ()));
		setDeparted (true);
		return true;
	}

	return false;
}

bool Flight::landNow (bool force)
{
	if (force || canLand ())
	{
		setLandingTime (nullSeconds (QDateTime::currentDateTime ().toUTC ()));
		setNumLandings (getNumLandings ()+1);
		setLanded (true);

		if (isNone (getLandingLocation ()))
			setLandingLocation (Settings::instance ().location);

		return true;
	}

	return false;
}

bool Flight::landTowflightNow (bool force)
{
	if (force || canTowflightLand ())
	{
		setTowflightLandingTime (nullSeconds (QDateTime::currentDateTime ().toUTC ()));
		setTowflightLanded (true);
		if (towflightLandsHere () && isNone (getTowflightLandingLocation ()))
			setTowflightLandingLocation (Settings::instance ().location);
		return true;
	}

	return false;
}

bool Flight::performTouchngo (bool force)
{
	if (force || canTouchngo ())
	{
		setNumLandings (getNumLandings ()+1);
		return true;
	}

	return false;
}


// ***********
// ** Times **
// ***********

// TODO remove
QDate Flight::effdatum (Qt::TimeSpec spec) const
{
	// TODO this assumes that every flight at least departs or lands here.
	return effectiveTime ().toTimeSpec (spec).date ();
}

QDate Flight::getEffectiveDate (Qt::TimeSpec spec, QDate defaultDate) const
{
	// TODO this assumes that every flight at least departs or lands here.
	if (departsHere () && getDeparted ())
		return getDepartureTime ().toTimeSpec (spec).date ();

	if (landsHere () && getLanded ())
		return getLandingTime ().toTimeSpec (spec).date ();

	return defaultDate;
}

QDateTime Flight::effectiveTime () const
{
	// TODO this assumes that every flight at least departs or lands here.
	if (departsHere () && getDeparted ()) return getDepartureTime ();
	if (landsHere () && getLanded ()) return getLandingTime ();
	return QDateTime ();
}

QTime Flight::flightDuration () const
{
	if (getDeparted () && getLanded ())
		return QTime ().addSecs (getDepartureTime ().secsTo (getLandingTime ()));
	else if (getDeparted ())
		return QTime ().addSecs (getDepartureTime ().secsTo (QDateTime::currentDateTime ().toUTC ()));
	else
		return QTime ();
}

QTime Flight::towflightDuration () const
{
	if (getDeparted () && getTowflightLanded ())
		return QTime ().addSecs (getDepartureTime ().secsTo (getTowflightLandingTime ()));
	else if (getDeparted ())
		return QTime ().addSecs (getDepartureTime ().secsTo (QDateTime::currentDateTime ().toUTC ()));
	else
		return QTime ();
}


// ********************
// ** Error checking **
// ********************

/**
 * This does not consider errors of the towflight
 *
 * @param cache
 * @return
 */
bool Flight::isErroneous (Cache &cache) const
{
	Plane *thePlane=cache.getNewObject<Plane> (getPlaneId ());
	LaunchMethod *theLaunchMethod=cache.getNewObject<LaunchMethod> (getLaunchMethodId ());

	bool erroneous=isErroneous (thePlane, NULL, theLaunchMethod);

	delete thePlane;
	delete theLaunchMethod;

	return erroneous;
}

/**
 * Determines whether the flight (not the towflight) is erroneous
 *
 * @param fz the plane, if known, or NULL
 * @param sfz the towplane, if any and known, or NULL
 * @param sa the launch method, if known, or NULL
 * @param errorText a description of the errors is written here if there is an
 *                  error
 * @return true if there is an error, false else
 */
bool Flight::isErroneous (Plane *fz, Plane *sfz, LaunchMethod *sa, QString *errorText) const
{
	// FIXME: stop after first error? use cache?
	QList<FlightError> errors=getErrors (false, fz, sfz, sa);
	if (errors.isEmpty ())
	{
		return false;
	}
	else
	{
		if (errorText) *errorText=errorDescription (errors.first ());
		return true;
	}
}

QString Flight::errorDescription (FlightError code) const
{
	switch (code)
	{
		// TODO tr for all; .arg
		case ff_ok: return utf8 ("Kein Fehler");
		case ff_keine_id: return utf8 ("Flug hat keine ID");
		case ff_kein_flugzeug: return utf8 ("Kein Flugzeug angegeben");
		// TODO use person_bezeichnung (flightType) (oder wie die heißt) here
		case ff_pilot_nur_nachname: return utf8 ("Für den %1 ist nur ein Nachname angegeben").arg (utf8 (getType ()==typeTraining2?"Flugschüler":"Piloten"));
		case ff_pilot_nur_vorname: return  utf8 ("Für den ")+utf8 (getType ()==typeTraining2?"Flugschüler":"Piloten")+" ist nur ein Vorname angegeben";
		case ff_pilot_nicht_identifiziert: return  "Der "+utf8 (getType ()==typeTraining2?"Flugschüler":"Pilot")+" ist nicht identifiziert";
		case ff_begleiter_nur_nachname: return utf8 ("Für den %1 ist nur ein Nachname angegeben").arg (QString (getType ()==typeTraining2?"Fluglehrer":"Begleiter"));
		case ff_begleiter_nur_vorname: return  utf8 ("Für den ")+QString (getType ()==typeTraining2?"Fluglehrer":"Begleiter")+" ist nur ein Vorname angegeben";
		case ff_begleiter_nicht_identifiziert: return  "Der "+QString (getType ()==typeTraining2?"Fluglehrer":"Begleiter")+" ist nicht identifiziert";
		case ff_towpilot_nur_nachname: return utf8 ("Für den Schleppiloten ist nur ein Nachname angegeben");
		case ff_towpilot_nur_vorname: return  utf8 ("Für den Schleppiloten ist nur ein Vorname angegeben");
		case ff_towpilot_nicht_identifiziert: return  "Der Schleppilot ist nicht identifiziert";
		case ff_kein_pilot: return utf8 ("Kein %1 angegeben").arg (utf8 (getType ()==typeTraining2 || getType ()==typeTraining1?"Flugschüler":"Pilot"));
		case ff_pilot_gleich_begleiter: return utf8 (getType ()==typeTraining2?"Flugschüler und Fluglehrer":"Pilot und Begleiter")+" sind identisch";
		case ff_pilot_gleich_towpilot: return utf8 (getType ()==typeTraining2?"Flugschüler":"Pilot")+" und Schlepppilot sind identisch";
		case ff_schulung_ohne_begleiter: return utf8 ("Doppelsitzige Schulung ohne Fluglehrer");
		case ff_begleiter_nicht_erlaubt: return utf8 ("Begleiter ist nicht erlaubt");
		case ff_nur_gelandet: return utf8 ("Flug ist gelandet, aber nicht gestartet");
		case ff_landung_vor_start: return utf8 ("Landung liegt vor Start");
		case ff_keine_startart: return utf8 ("Keine Startart angegeben");
		case ff_kein_modus: return utf8 ("Kein Modus angegeben");
		case ff_kein_sfz_modus: return utf8 ("Kein Modus für den Schleppflug angegeben");
		case ff_kein_flugtyp: return utf8 ("Kein Flugtyp angegeben");
		case ff_landungen_negativ: return utf8 ("Negative Anzahl Landungen");
		case ff_landungen_null: return utf8 ("Flug ist gelandet, aber Anzahl der Landungen ist 0");
		case ff_schlepp_nur_gelandet: return utf8 ("Schleppflug ist gelandet, aber nicht gestartet");
		case ff_schlepp_landung_vor_start: return utf8 ("Landung des Schleppflugs liegt vor Start");
		case ff_doppelsitzige_schulung_in_einsitzer: return utf8 ("Doppelsitzige Schulung in Einsitzer");
		case ff_kein_startort: return utf8 ("Kein Startort angegeben");
		case ff_kein_zielort: return utf8 ("Kein Zielort angegeben");
		case ff_kein_zielort_sfz: return utf8 ("Kein Zielort für das Schleppflugzeug angegeben");
		case ff_segelflugzeug_landungen: return utf8 ("Segelflugzeug macht mehr als eine Landung");
		case ff_segelflugzeug_landungen_ohne_landung: return utf8 ("Segelflugzeug macht Landungen ohne Landezeit");
		case ff_begleiter_in_einsitzer: return utf8 ("Begleiter in einsitzigem Flugzeug");
		case ff_gastflug_in_einsitzer: return utf8 ("Gastflug in einsitzigem Flugzeug");
		case ff_segelflugzeug_selbststart: return utf8 ("Segelflugzeug im Selbststart");
		case ff_landungen_ohne_start: return utf8 ("Anzahl Landungen ungleich null ohne Start");
		case ff_startort_gleich_zielort: return utf8 ("Startort gleich Zielort");
		case ff_kein_schleppflugzeug: return utf8 ("Schleppflugzeug nicht angegeben");
		case ff_towplane_is_glider: return utf8 ("Schleppflugzeug ist Segelflugzeug");
		// No default to allow compiler warning
	}

	return "Unbekannter Fehler";
}

void Flight::checkPerson (QList<FlightError> &errors, dbId id, const QString &lastName, const QString &firstName, bool required,
	FlightError notSpecifiedError, FlightError lastNameOnlyError, FlightError firstNameOnlyError, FlightError notIdentifiedError) const
{
	// Person specified - no error
	if (idValid (id)) return;

	bool  lastNameSpecified=!isBlank ( lastName);
	bool firstNameSpecified=!isBlank (firstName);

	if (lastNameSpecified && firstNameSpecified)
		// Both specified
		errors << notIdentifiedError;
	else if (lastNameSpecified)
		// Last name specified only
		errors << lastNameOnlyError;
	else if (firstNameSpecified)
		// First name specified only
		errors << firstNameOnlyError;
	else
	{
		// None specified
		if (required)
			errors << notSpecifiedError;
	}
}

QList<FlightError> Flight::getErrors (bool includeTowflightErrors, Plane *plane, Plane *towplane, LaunchMethod *launchMethod) const
{
	if (!cachedErrorsValid)
	{
		cachedErrors=getErrorsImpl (includeTowflightErrors, plane, towplane, launchMethod);
		cachedErrorsValid=true;
	}

	return cachedErrors;
}

// FIXME pass cache instead
QList<FlightError> Flight::getErrorsImpl (bool includeTowflightErrors, Plane *plane, Plane *towplane, LaunchMethod *launchMethod) const
{
	// TODO einsitzige Schulung mit Begleiter

	QList<FlightError> errors;

	// Basic properties
	if (idInvalid (getId ())) errors << ff_keine_id;
	if (getType ()==typeNone) errors << ff_kein_flugtyp;

	// Pilot
	checkPerson (errors, getPilotId (), getPilotLastName (), getPilotFirstName (),
		launchMethod && launchMethod->personRequired,
		ff_kein_pilot, ff_pilot_nur_nachname, ff_pilot_nur_vorname, ff_pilot_nicht_identifiziert);

	// Copilot (if recorded)
	if (typeCopilotRecorded (getType ()))
	{
		checkPerson (errors, getCopilotId (), getCopilotLastName (), getCopilotFirstName (),
			false,
			ff_ok, ff_begleiter_nur_nachname, ff_begleiter_nur_vorname, ff_begleiter_nicht_identifiziert
			);

		if (idValid (getPilotId ()) && getPilotId ()==getCopilotId ())
			errors << ff_pilot_gleich_begleiter;

		if (getType ()==typeTraining2 && !copilotSpecified ())
			errors << ff_schulung_ohne_begleiter;
	}

	// Towpilot (if recorded)
	if (Settings::instance ().recordTowpilot && launchMethod && launchMethod->isAirtow ())
	{
		checkPerson (errors, getTowpilotId (), getTowpilotLastName (), getTowpilotFirstName (),
			false,
			ff_ok, ff_towpilot_nur_nachname, ff_towpilot_nur_vorname, ff_towpilot_nicht_identifiziert);

		if (idValid (getTowpilotId ()) && getPilotId ()==getTowpilotId ())
			errors << ff_pilot_gleich_towpilot;
	}
	// TODO copilot equals towpilot, if both are recorded

	// Plane
	if (idInvalid (getPlaneId ())) errors << ff_kein_flugzeug;

	if (plane)
	{
		// Single-seat planes
		if (plane->numSeats==1)
		{
			if (getType ()==typeTraining2    ) errors << ff_doppelsitzige_schulung_in_einsitzer;
			if (getType ()==typeGuestPrivate ) errors << ff_gastflug_in_einsitzer;
			if (getType ()==typeGuestExternal) errors << ff_gastflug_in_einsitzer;

			if (typeCopilotRecorded (getType ()) && copilotSpecified ())
				errors << ff_begleiter_in_einsitzer;
		}

		// Gliders
		if (plane->category==Plane::categoryGlider)
		{
			if (launchMethod && launchMethod->type==LaunchMethod::typeSelf)
				errors << ff_segelflugzeug_selbststart;
		}

		// Gliders (except airtows)
		if (plane->category==Plane::categoryGlider && launchMethod && !launchMethod->isAirtow ())
		{
			if (getNumLandings ()>1)                  errors << ff_segelflugzeug_landungen;
			if (getNumLandings ()>0 && !getLanded ()) errors << ff_segelflugzeug_landungen_ohne_landung;
		}
	}

	// Status - generic
	if (getNumLandings ()<0) errors << ff_landungen_negativ;

	if (departsHere ()!=landsHere () && getDepartureLocation ()==getLandingLocation ())
		errors << ff_startort_gleich_zielort;

	// Status - local flights
	if (departsHere () && landsHere ())
	{
		if (!getDeparted () && getLanded ())
			errors << ff_nur_gelandet;

		if (getDeparted () && getLanded () && getDepartureTime ()>getLandingTime ())
			errors << ff_landung_vor_start;
	}

	// Status - departing flights (local or leaving)
	if (departsHere ())
	{
		if (getDeparted () && idInvalid (getLaunchMethodId ()) && !isTowflight ())
			errors << ff_keine_startart;

		if (!getDeparted () && getNumLandings ()>0)
			errors << ff_landungen_ohne_start;
	}

	// Status - landing flights (local or coming)
	if (landsHere ())
	{
		if (getNumLandings ()==0 && getLanded ())
			errors << ff_landungen_null;
	}

	if ((getDeparted () || !departsHere ())
		&& isNone (getDepartureLocation ()))
		errors << ff_kein_startort;

	if ((getLanded () || !landsHere ())
		&& isNone (getLandingLocation ()))
		errors << ff_kein_zielort;

	// Towflight errors
	if (includeTowflightErrors && launchMethod && launchMethod->isAirtow ())
	{
		// FIXME implement
		if ((getTowflightLanded () || !towflightLandsHere ())
			&& isNone (getTowflightLandingLocation ()))
			errors << ff_kein_zielort_sfz;

		if (!launchMethod->towplaneKnown () && idInvalid (getTowplaneId ()))
			errors << ff_kein_schleppflugzeug;

		if (!launchMethod->towplaneKnown () && towplane
			&& towplane->category==Plane::categoryGlider)
			errors << ff_towplane_is_glider;
	}

	return errors;
}


// ****************
// ** Formatting **
// ****************

QString personToString (dbId id, QString lastName, QString firstName)
{
	if (idValid (id))
		return QString::number (id);
	else if (isNone(lastName) && isNone(firstName))
		return "-";
	else
		return QString ("(%1, %2)")
			.arg (isNone (lastName)?QString ("?"):lastName)
			.arg (isNone (firstName)?QString ("?"):firstName);
}

QString timeToString (bool performed, QDateTime time)
{
	if (performed)
		return time.toUTC ().time ().toString ("h:mm") +"Z";
	else
		return "-";
}

QString Flight::toString () const
{
	return QString ("id=%1, plane=%2, type=%3, pilot=%4, copilot=%5, mode=%6, "
		"launchMethod=%7, towplane=%8, towpilot=%9, towFlightMode=%10, "
		"departureTime=%11, landingTime=%12, towflightLandingTime=%13, "
		"departureLocation='%14', landingLocation='%15', towflightLandingLocation='%16', "
		"numLandings=%17, comment='%18', accountingNote='%19'")

		.arg (getId ())
		.arg (getPlaneId ())
		.arg (shortTypeText (getType ()))
		.arg (personToString (getPilotId (), getPilotLastName (), getPilotFirstName ()))
		.arg (personToString (getCopilotId (), getCopilotLastName (), getCopilotFirstName ()))
		.arg (modeText (getMode ()))

		.arg (getLaunchMethodId ())
		.arg (getTowplaneId ())
		.arg (personToString (getTowpilotId (), getTowpilotLastName (), getTowpilotFirstName ()))
		.arg (modeText (getTowflightMode ()))

		.arg (timeToString (getDeparted (), getDepartureTime ()))
		.arg (timeToString (getLanded (), getLandingTime ()))
		.arg (timeToString (getTowflightLanded (), getTowflightLandingTime ()))

		.arg (getDepartureLocation ())
		.arg (getLandingLocation ())
		.arg (getTowflightLandingLocation ())

		.arg (getNumLandings ())
		.arg (getComments ())
		.arg (getAccountingNotes ())
		;
}


// **********
// ** Misc **
// **********

/**
 * Creates the towflight for the current flights. Assumes that the flight is an
 * airtow.
 *
 * @param theTowplaneId the ID of the towplane
 * @param towLaunchMethod the ID of the launch method for the towflight. May be
 *                        an invalid ID if the methods processing the towflight
 *                        recognize the towflight by its type and assume a self
 *                        launch in this case.
 * @return
 */
Flight Flight::makeTowflight (dbId theTowplaneId, dbId towLaunchMethod) const
{
	Flight towflight;

	// The tow flight gets the same ID because there would be no way to get
	// the ID for a given tow flight. The tow flight can be distinguished
	// from the real flight by the flight type (by calling isTowflight ()).
	towflight.setId (getId ());

	// Always use the towplane ID passed by the caller, even if it is invalid -
	// this means that the towplane specified in the launch method was not found.
	towflight.setPlaneId (theTowplaneId);

	// The towflight's pilot is our towpilot and there is no copilot or towpilot
	towflight.setPilotId (getTowpilotId ());
	towflight.setCopilotId (invalidId);
	towflight.setTowpilotId (invalidId);

	towflight.setDepartureTime (getDepartureTime ());      // The tow flight departed the same time as the towed flight.
	towflight.setLandingTime (getTowflightLandingTime ()); // The tow flight landing time is our landingTimeTowflight.
	towflight.setTowflightLandingTime (QDateTime ());      // The tow flight has no tow flight.

	// The launchMethod of the tow flight is given as a parameter.
	towflight.setLaunchMethodId (towLaunchMethod);

	towflight.setType (typeTow);
	towflight.setDepartureLocation (getDepartureLocation ());      // The tow flight departed the same location as the towed flight.
	towflight.setLandingLocation (getTowflightLandingLocation ()); // The tow flight landing place is our landingLocationTowplane.
	towflight.setTowflightLandingLocation ("");

	towflight.setNumLandings ((towflightLandsHere () && getTowflightLanded ())?1:0);

	towflight.setComments (utf8 ("Schleppflug für Flug Nr. %1").arg (getId ()));
	towflight.setAccountingNotes ("");
	towflight.setMode (getTowflightMode ());
	towflight.setTowflightMode (modeLocal);
	towflight.setPilotLastName (getTowpilotLastName ());
	towflight.setPilotFirstName (getTowpilotFirstName ());
	towflight.setCopilotLastName ("");
	towflight.setCopilotFirstName ("");
	towflight.setTowpilotLastName ("");
	towflight.setTowpilotFirstName ("");
	towflight.setTowplaneId (invalidId);
	towflight.setDeparted (getDeparted ());
	towflight.setLanded (getTowflightLanded ());
	towflight.setTowflightLanded (false);

	return towflight;
}

QList<Flight> Flight::makeTowflights (const QList<Flight> &flights, Cache &cache)
{
	QList<Flight> towflights;

	// The launch method is the same for all tow flights
	dbId towLaunchMethod=cache.getLaunchMethodByType (LaunchMethod::typeSelf);

	// Create a towflight for each flight which is an airtow
	foreach (const Flight &flight, flights)
		if (flight.isAirtow (cache))
			towflights.append (flight.makeTowflight (flight.effectiveTowplaneId (cache), towLaunchMethod));

	return towflights;
}


// TODO move to PlaneLog
bool Flight::collectiveLogEntryPossible (const Flight *prev, const Plane *plane) const
{
	// Only allow if the previous flight and the current flight departs and lands
	// at the same place.
	if (prev->getMode ()!=modeLocal || getMode ()!=modeLocal) return false;
	if (prev->getDepartureLocation ().trimmed ().toLower ()!=prev->getLandingLocation ().trimmed ().toLower ()) return false;
	if (prev->  getLandingLocation ().trimmed ().toLower ()!=    getDepartureLocation ().trimmed ().toLower ()) return false;
	if (      getDepartureLocation ().trimmed ().toLower ()!=      getLandingLocation ().trimmed ().toLower ()) return false;

	// For motor planes: only allow if the flights are towflights.
	// Unknown planes are treated like motor planes.
	if (plane && (plane->category==Plane::categoryGlider || plane->category==Plane::categoryMotorglider))
	{
		return true;
	}
	else
	{
		if (prev->isTowflight () && isTowflight ()) return true;
		return false;
	}
}


// *******************
// ** SQL interface **
// *******************

QString Flight::dbTableName ()
{
	return "flights";
}

QString Flight::selectColumnList ()
{
	return
		"id,pilot_id,copilot_id,plane_id,type,mode,departed,landed,towflight_landed" // 9
		",launch_method_id,departure_location,landing_location,num_landings,departure_time,landing_time" // 6 Σ15
		",pilot_last_name,pilot_first_name,copilot_last_name,copilot_first_name" // 4 Σ19
		",towflight_landing_time,towflight_mode,towflight_landing_location,towplane_id" // 4 Σ23
		",accounting_notes,comments" // 2 Σ25
		",towpilot_id,towpilot_last_name,towpilot_first_name" // 3 Σ28
		;
}

Flight Flight::createFromResult (const Result &result)
{
	Flight f (result.value (0).toLongLong ());

	f.setPilotId          (result.value (1).toLongLong ());
	f.setCopilotId        (result.value (2).toLongLong ());
	f.setPlaneId          (result.value (3).toLongLong ());
	f.setType             (typeFromDb (
	                       result.value (4).toString   ()));
	f.setMode             (modeFromDb (
	                       result.value (5).toString   ()));
	f.setDeparted         (result.value (6).toBool     ());
	f.setLanded           (result.value (7).toBool     ());
	f.setTowflightLanded  (result.value (8).toBool     ());

	f.setLaunchMethodId    (result.value ( 9).toLongLong ());
	f.setDepartureLocation (result.value (10).toString   ());
	f.setLandingLocation   (result.value (11).toString   ());
	f.setNumLandings       (result.value (12).toInt      ());
	f.setDepartureTime     (result.value (13).toDateTime ()); f.refToDepartureTime ().setTimeSpec (Qt::UTC); // not toUTC
	f.setLandingTime       (result.value (14).toDateTime ()); f.refToLandingTime ().setTimeSpec (Qt::UTC); // not toUTC

	f.setPilotLastName    (result.value (15).toString ());
	f.setPilotFirstName   (result.value (16).toString ());
	f.setCopilotLastName  (result.value (17).toString ());
	f.setCopilotFirstName (result.value (18).toString ());

	f.setTowflightLandingTime     (result.value (19).toDateTime ()); f.refToTowflightLandingTime ().setTimeSpec (Qt::UTC); // not toUTC
	f.setTowflightMode            (modeFromDb (
	                               result.value (20).toString   ()));
	f.setTowflightLandingLocation (result.value (21).toString   ());
	f.setTowplaneId               (result.value (22).toLongLong ());

	f.setAccountingNotes (result.value (23).toString ());
	f.setComments        (result.value (24).toString ());

	f.setTowpilotId         (result.value (25).toLongLong ());
	f.setTowpilotLastName   (result.value (26).toString   ());
	f.setTowpilotFirstName  (result.value (27).toString   ());

	return f;
}

QString Flight::insertColumnList ()
{
	return
		"pilot_id,copilot_id,plane_id,type,mode,departed,landed,towflight_landed" // 8
		",launch_method_id,departure_location,landing_location,num_landings,departure_time,landing_time" // 6 Σ14
		",pilot_last_name,pilot_first_name,copilot_last_name,copilot_first_name" // 4 Σ18
		",towflight_landing_time,towflight_mode,towflight_landing_location,towplane_id" // 4 Σ22
		",accounting_notes,comments" // 2 Σ24
		",towpilot_id,towpilot_last_name,towpilot_first_name" // 3 Σ27
		;
}

QString Flight::insertPlaceholderList ()
{
	return
		"?,?,?,?,?,?,?,?"
		",?,?,?,?,?,?"
		",?,?,?,?"
		",?,?,?,?"
		",?,?"
		",?,?,?"
		;
}

void Flight::bindValues (Query &q) const
{
	q.bind (getPilotId ());
	q.bind (getCopilotId ());
	q.bind (getPlaneId ());
	q.bind (typeToDb (getType ()));
	q.bind (modeToDb (getMode ()));
	q.bind (getDeparted ());
	q.bind (getLanded ());
	q.bind (getTowflightLanded ());

	q.bind (getLaunchMethodId ());
	q.bind (getDepartureLocation ());
	q.bind (getLandingLocation ());
	q.bind (getNumLandings ());
	q.bind (getDepartureTime ().toUTC  ());
	q.bind (getLandingTime ().toUTC  ());

	q.bind (getPilotLastName ());
	q.bind (getPilotFirstName ());
	q.bind (getCopilotLastName ());
	q.bind (getCopilotFirstName ());

	q.bind (getTowflightLandingTime ().toUTC  ());
	q.bind (modeToDb (getTowflightMode ()));
	q.bind (getTowflightLandingLocation ());
	q.bind (getTowplaneId ());

	q.bind (getAccountingNotes ());
	q.bind (getComments ());

	q.bind (getTowpilotId ());
	q.bind (getTowpilotLastName ());
	q.bind (getTowpilotFirstName ());
}

QList<Flight> Flight::createListFromResult (Result &result)
{
	QList<Flight> list;

	while (result.next ())
		list.append (createFromResult (result));

	return list;
}


// *** Enum mappers

QString Flight::modeToDb (Flight::Mode mode)
{
	switch (mode)
	{
		case modeLocal   : return "local";
		case modeComing  : return "coming";
		case modeLeaving : return "leaving";
		// no default
	}

	assert (false);
	return "?";
}

Flight::Mode Flight::modeFromDb (QString mode)
{
	if      (mode=="local"  ) return modeLocal;
	else if (mode=="coming" ) return modeComing;
	else if (mode=="leaving") return modeLeaving;
	else                      return modeLocal;
}

QString Flight::typeToDb (Type type)
{
	switch (type)
	{
		case typeNone          : return "?";
		case typeNormal        : return "normal";
		case typeTraining2     : return "training_2";
		case typeTraining1     : return "training_1";
		case typeTow           : return "tow";
		case typeGuestPrivate  : return "guest_private";
		case typeGuestExternal : return "guest_external";
		// no default
	};

	assert (false);
	return "?";
}

Flight::Type Flight::typeFromDb (QString type)
{
	if      (type=="normal"        ) return typeNormal;
	else if (type=="training_2"    ) return typeTraining2;
	else if (type=="training_1"    ) return typeTraining1;
	else if (type=="tow"           ) return typeTow;
	else if (type=="guest_private" ) return typeGuestPrivate;
	else if (type=="guest_external") return typeGuestExternal;
	else                             return typeNone;
}

Query Flight::referencesPersonCondition (dbId id)
{
	return Query ("pilot_id=? OR copilot_id=? OR towpilot_id=?")
		.bind (id).bind (id).bind (id);
}

Query Flight::referencesPlaneCondition (dbId id)
{
	return Query ("plane_id=? OR towplane_id=?")
		.bind (id).bind (id);
}

Query Flight::referencesLaunchMethodCondition (dbId id)
{
	return Query ("launch_method_id=?")
		.bind (id);
}


QColor Flight::getColor (Cache &cache) const
{
	if (!cachedColor.isValid ())
	{
		cachedColor=flightColor (getMode (), isErroneous (cache), isTowflight (), getDeparted (), getLanded ());
	}

	return cachedColor;
}


// ***********
// ** Cache **
// ***********

void Flight::dataChanged ()
{
	cachedColor=QColor ();
	cachedErrorsValid=false;
}
