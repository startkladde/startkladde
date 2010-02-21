#include "Flight.h"

#include <iostream>

#include "src/config/Options.h"
#include "src/db/DataStorage.h"
#include "src/model/Plane.h"
#include "src/model/LaunchMethod.h"
#include "src/text.h"

// TODO Vereinheitlichen der Statusfunktionen untereinander und mit den
// condition-strings
// TODO Errors in other places: for towflights, the landing time is meaningful
// even if !landsHere.
// TODO consider an AbstractFlight and a TowFlightProxy

/*
 * Potential model changes:
 *   - for airtows, always store the towplane ID here and only store a generic
 *     "airtow" launch method
 *   - store the towplane ID or the registration?
 */


// ******************
// ** Construction **
// ******************

Flight::Flight ()
{
	initialize (invalid_id);
}

Flight::Flight (db_id id)
{
	initialize (id);
}

void Flight::initialize (db_id id)
{
	this->id=id;

	planeId         =invalid_id;
	numLandings     =invalid_id;
	pilotId         =invalid_id;
	copilotId       =invalid_id;
	towpilotId      =invalid_id;
	launchMethodId  =invalid_id;
	towplaneId      =invalid_id;

	type          =typeNone;
	mode          =modeNone;
	towflightMode =modeNone;

	departed        =false;
	landed          =false;
	towflightLanded =false;
}


// ****************
// ** Comparison **
// ****************

bool Flight::operator< (const Flight &o) const
{
	return sort (&o)<0;
}

/**
 * Compares the flight with another flight.
 *
 * @return >0 if this flight is later, <0 if this flight is earlier
 */
int Flight::sort (const Flight *other) const
{
	// Both prepared
	if (isPrepared () && other->isPrepared  ())
	{
		// Incoming prepared before local launching prepared
		if (departsHere () && !other->departsHere ()) return 1;
		if (!departsHere () && other->departsHere ()) return -1;

		// Flights are equal
		return 0;
	}

	// Prepared flights to the end
	if (isPrepared ()) return 1;
	if (other->isPrepared ()) return -1;

	// Sort by effective time
	Time t1=effectiveTime ();
	Time t2=other->effectiveTime ();
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
	if (departsHere () && departed) return true;
	if (landsHere () && landed) return true;
	return false;
}

bool Flight::finished () const
{
	if (isTowflight ())
		// For leaving towflights, landed means ended.
		return landed;
	else
		return (landsHere ()?landed:departed);
}


// **********
// ** Crew **
// **********

QString Flight::pilotDescription () const
{
	return typePilotDescription (type);
}

QString Flight::copilotDescription () const
{
	return typeCopilotDescription (type);
}

bool Flight::pilotSpecified () const
{
	return id_valid (pilotId) ||
		!eintraege_sind_leer (pilotFirstName, pilotLastName);
}

bool Flight::copilotSpecified () const
{
	return id_valid (copilotId) ||
		!eintraege_sind_leer (copilotFirstName, copilotLastName);
}

bool Flight::towpilotSpecified () const
{
	return id_valid (towpilotId) ||
		!eintraege_sind_leer (towpilotFirstName, towpilotLastName);
}


QString Flight::incompletePilotName () const
{
	return incompletePersonName (pilotLastName, pilotFirstName);
}

QString Flight::incompleteCopilotName () const
{
	return incompletePersonName (copilotLastName, copilotFirstName);
}

QString Flight::incompleteTowpilotName () const
	/*
	 * Makes the incomplete name of the towpilot.
	 * Return value:
	 *   - the name.
	 */
{
	return incompletePersonName (towpilotLastName, towpilotFirstName);
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
	if (eintrag_ist_leer (nn) && eintrag_ist_leer (vn)) return ("-");
	else if (eintrag_ist_leer (nn)) return QString ("(???, %1)").arg (vn);
	else if (eintrag_ist_leer (vn)) return QString ("(%1, %2)").arg (nn).arg ("???"); // ??) would be a trigraph
	else                            return QString ("%1, %2").arg (nn).arg (vn);
}


// ***********************
// ** Departure/landing **
// ***********************

#define notPossibleIf(condition, reasonText) do { if (condition) { if (reason) *reason=reasonText; return false; } } while (0)

bool Flight::canDepart (QString *reason) const
{
	// TODO only for flights of today

	// Already landed
	notPossibleIf (landsHere () && landed, "Der Flug ist bereits gelandet");

	// Does not start here
	notPossibleIf (!departsHere (), "Der Flug startet nicht hier.");

	// Already started
	notPossibleIf (departed, "Der Flug ist bereits gestartet.");

	return true;
}

bool Flight::canLand (QString *reason) const
{
	// TODO only for flights of today

	// Already landed
	notPossibleIf (landed, "Der Flug ist bereits gelandet.");

	// Does not land here (only applies to non-towflights)
	notPossibleIf (!isTowflight () && !landsHere (), "Der Flug landet nicht hier.");

	// Must start first
	notPossibleIf (departsHere () && !departed, "Der Flug ist noch nicht gestartet.");

	return true;
}

bool Flight::canTouchngo (QString *reason) const
{
	// TODO only for flights of today

	// Towflight
	notPossibleIf (isTowflight (), "Der Flug ist ein Schleppflug");

	// Already landed
	notPossibleIf (landed, "Der Flug ist bereits gelandet.");

	// Must start first
	notPossibleIf (departsHere () && !departed, "Der Flug ist noch nicht gestartet.");

	return true;
}

bool Flight::canTowflightLand (QString *reason) const
{
	// Already landed
	notPossibleIf (towflightLanded, "Der Schleppflug ist bereits gelandet.");

	// Must start first
	notPossibleIf (departsHere () && !departed, "Der Flug ist noch nicht gestartet.");

	return true;
}

#undef notPossibleIf

bool Flight::departNow (bool force)
{
	if (force || canDepart ())
	{
		departureTime.set_current (true);
		departed=true;
		return true;
	}

	return false;
}

bool Flight::landNow (bool force)
{
	if (force || canLand ())
	{
		landingTime.set_current (true);
		numLandings++;
		landed=true;

		if (eintrag_ist_leer (landingLocation))
			landingLocation=opts.ort;

		return true;
	}

	return false;
}

bool Flight::landTowflightNow (bool force)
{
	if (force || canTowflightLand ())
	{
		towflightLandingTime.set_current (true);
		towflightLanded=true;
		if (towflightLandsHere () && eintrag_ist_leer (towflightLandingLocation)) towflightLandingLocation=opts.ort;
		return true;
	}

	return false;
}

bool Flight::performTouchngo (bool force)
{
	if (force || canTouchngo ())
	{
		numLandings++;
		return true;
	}

	return false;
}


// ***********
// ** Times **
// ***********

QDate Flight::effdatum (time_zone tz) const
{
        return effectiveTime ().get_qdate (tz);
}

QDate Flight::getEffectiveDate (time_zone tz, QDate defaultDate) const
{
	// TODO this assumes that every flight at least starts or lands here.
	if (departsHere () && departed)
		return departureTime.get_qdate (tz);

	if (landsHere () && landed)
		return landingTime.get_qdate (tz);

	return defaultDate;
}

Time Flight::effectiveTime () const
{
	// TODO this assumes that every flight at least starts or lands here.
	if (departsHere () && departed) return departureTime;
	if (landsHere () && landed) return landingTime;
	return Time ();
}

Time Flight::flightDuration () const
{
	Time t;
	if (departed && landed)
		t.set_to (departureTime.secs_to (&landingTime));
	else
	{
		Time now;
		now.set_current(true);
		t.set_to (departureTime.secs_to (&now));
	}

	return t;
}

Time Flight::towflightDuration () const
{
	Time t;
	if (departed && towflightLanded)
		t.set_to (departureTime.secs_to (&towflightLandingTime));
	else
	{
		Time now;
		now.set_current(true);
		t.set_to (departureTime.secs_to (&now));
	}

	return t;
}


// ********************
// ** Error checking **
// ********************

/**
 * This uses fehlerhaft, does not consider schlepp_fehlerhaft
 *
 * @param dataStorage
 * @return
 */
bool Flight::isErroneous (DataStorage &dataStorage) const
{
	Plane *thePlane=dataStorage.getNewObject<Plane> (planeId);
	LaunchMethod *theLaunchMethod=dataStorage.getNewObject<LaunchMethod> (launchMethodId);

	bool erroneous=fehlerhaft (thePlane, NULL, theLaunchMethod);

	delete thePlane;
	delete theLaunchMethod;

	return erroneous;
}

/**
 * Determines whether the flight is erroneous
 *
 * @param fz the plane, if known, or NULL
 * @param sfz the towplane, if any and known, or NULL
 * @param sa the launch method, if known, or NULL
 * @param errorText a description of the errors is written here if there is an
 *                  error
 * @return true if there is an error, false else
 */
bool Flight::fehlerhaft (Plane *fz, Plane *sfz, LaunchMethod *sa, QString *errorText) const
{
	int i=0;
	FlightError error=errorCheck (&i, true, false, fz, sfz, sa);

	if (errorText)
		if (error!=ff_ok)
			*errorText=errorDescription(error);

	return (error!=ff_ok);
}

// TODO replace by checking the towflight?
bool Flight::schlepp_fehlerhaft (Plane *fz, Plane *sfz, LaunchMethod *sa, QString *errorText) const
	/*
	 * Finds out if the towflight for this flight (if any) contains an error.
	 * Parameters:
	 *   - fz: the plane data structure for the flight.
	 * Return value:
	 *   - true if any error was found
	 *   - false else.
	 */
{
	int i=0;
	FlightError error=errorCheck (&i, false, true, fz, sfz, sa);

	if (errorText)
		if (error!=ff_ok)
			*errorText=errorDescription(error);

	return (error!=ff_ok);
}

QString Flight::errorDescription (FlightError code) const
{
	switch (code)
	{
		// TODO Utf8 for all
		case ff_ok: return QString::fromUtf8 ("Kein Fehler");
		case ff_keine_id: return QString::fromUtf8 ("Flug hat keine ID");
		case ff_kein_flugzeug: return QString::fromUtf8 ("Kein Flugzeug angegeben");
		// TODO use person_bezeichnung (flightType) (oder wie die heißt) here
		case ff_pilot_nur_nachname: return QString::fromUtf8 ("Für den "+QString (type==typeTraining2?"Flugschüler":"Piloten")+" ist nur ein Nachname angegeben");
		case ff_pilot_nur_vorname: return  "Für den "+QString (type==typeTraining2?"Flugschüler":"Piloten")+" ist nur ein Vorname angegeben";
		case ff_pilot_nicht_identifiziert: return  "Der "+QString (type==typeTraining2?"Flugschüler":"Pilot")+" ist nicht identifiziert";
		case ff_begleiter_nur_nachname: return QString::fromUtf8 ("Für den "+QString (type==typeTraining2?"Fluglehrer":"Begleiter")+" ist nur ein Nachname angegeben");
		case ff_begleiter_nur_vorname: return  "Für den "+QString (type==typeTraining2?"Fluglehrer":"Begleiter")+" ist nur ein Vorname angegeben";
		case ff_begleiter_nicht_identifiziert: return  "Der "+QString (type==typeTraining2?"Fluglehrer":"Begleiter")+" ist nicht identifiziert";
		case ff_towpilot_nur_nachname: return QString::fromUtf8 ("Für den Schleppiloten ist nur ein Nachname angegeben");
		case ff_towpilot_nur_vorname: return  "Für den Schleppiloten ist nur ein Vorname angegeben";
		case ff_towpilot_nicht_identifiziert: return  "Der Schleppilot ist nicht identifiziert";
		case ff_kein_pilot: return QString::fromUtf8 ("Kein "+QString (type==typeTraining2 || type==typeTraining1?"Flugschüler":"Pilot")+" angegeben");
		case ff_pilot_gleich_begleiter: return QString (type==typeTraining2?"Flugschüler und Fluglehrer":"Pilot und Begleiter")+" sind identisch";
		case ff_pilot_gleich_towpilot: return QString (type==typeTraining2?"Flugschüler":"Pilot")+" und Schlepppilot sind identisch";
		case ff_schulung_ohne_begleiter: return QString::fromUtf8 ("Doppelsitzige Schulung ohne Fluglehrer");
		case ff_begleiter_nicht_erlaubt: return QString::fromUtf8 ("Begleiter ist nicht erlaubt");
		case ff_nur_gelandet: return QString::fromUtf8 ("Flug ist gelandet, aber nicht gestartet");
		case ff_landung_vor_start: return QString::fromUtf8 ("Landung liegt vor Start");
		case ff_keine_startart: return QString::fromUtf8 ("Keine Startart angegeben");
		case ff_kein_modus: return QString::fromUtf8 ("Kein Modus angegeben");
		case ff_kein_sfz_modus: return QString::fromUtf8 ("Kein Modus für den Schleppflug angegeben");
		case ff_kein_flugtyp: return QString::fromUtf8 ("Kein Flugtyp angegeben");
		case ff_landungen_negativ: return QString::fromUtf8 ("Negative Anzahl Landungen");
		case ff_landungen_null: return QString::fromUtf8 ("Flug ist gelandet, aber Anzahl der Landungen ist 0");
		case ff_schlepp_nur_gelandet: return QString::fromUtf8 ("Schleppflug ist gelandet, aber nicht gestartet");
		case ff_schlepp_landung_vor_start: return QString::fromUtf8 ("Landung des Schleppflugs liegt vor Start");
		case ff_doppelsitzige_schulung_in_einsitzer: return QString::fromUtf8 ("Doppelsitzige Schulung in Einsitzer");
		case ff_kein_startort: return QString::fromUtf8 ("Kein Startort angegeben");
		case ff_kein_zielort: return QString::fromUtf8 ("Kein Zielort angegeben");
		case ff_kein_zielort_sfz: return QString::fromUtf8 ("Kein Zielort für das Schleppflugzeug angegeben");
		case ff_segelflugzeug_landungen: return QString::fromUtf8 ("Segelflugzeug macht mehr als eine Landung");
		case ff_segelflugzeug_landungen_ohne_landung: return QString::fromUtf8 ("Segelflugzeug macht Landungen ohne Landezeit");
		case ff_begleiter_in_einsitzer: return QString::fromUtf8 ("Begleiter in einsitzigem Flugzeug");
		case ff_gastflug_in_einsitzer: return QString::fromUtf8 ("Gastflug in einsitzigem Flugzeug");
		case ff_segelflugzeug_selbststart: return QString::fromUtf8 ("Segelflugzeug im Selbststart");
		case ff_landungen_ohne_start: return QString::fromUtf8 ("Anzahl Landungen ungleich null ohne Start");
		case ff_startort_gleich_zielort: return QString::fromUtf8 ("Startort gleich Zielort");
		case ff_kein_schleppflugzeug: return QString::fromUtf8 ("Schleppflugzeug nicht angegeben");
		case ff_towplane_is_glider: return QString::fromUtf8 ("Schleppflugzeug ist Segelflugzeug");
		// No default to allow compiler warning
	}

	return "Unbekannter Fehler";
}

/**
 * Performes the unified error checking.
 *
 * Usage of this function:
 *   - set *index=0.
 *   - call in loop until return value is ff_ok.
 *   - each call returns an error of ff_ok.
 *   - don't change the data in the middle or the results *may* be invalid.
 *   - you can stop the loop at any time.
 *   - the function is reentrant, provided you use different '*index'es.
 *   - you shouldn't use the value of the index variable, it is internal.
 *   - you should not set index to anything but 0 manually or the result is
 *     undefined.
 *
 * @param index the index of the first error to check
 * @param check_flug whether to theck the flight proper
 * @param check_schlepp whether to check the towflight
 * @param fz the plane, if known, or NULL
 * @param sfz the towplane, if any and known, or NULL
 * @param sa the launch method, if any and known, or nULL
 * @return an error code, or ff_ok
 */
FlightError Flight::errorCheck (int *index, bool check_flug, bool check_schlepp, Plane *fz, Plane *sfz, LaunchMethod *sa) const
{
	// TODO return a QList instead
	// TODO check_schlepp not used. Investigate.
	(void)check_schlepp;
#define CHECK_FEHLER(bereich, bedingung, fehlercode) if ((*index)==(num++)) { (*index)++; if (bereich && bedingung) return fehlercode; }
#define FLUG (check_flug)
#define SCHLEPP (check_schlepp)
	//printf ("Fehlercheckung: %d %s %s\n", *index, check_flug?"flug":"!flug", check_schlepp?"schlepp":"!schlepp");
	int num=0;

	// Note: when adding an error check concerning people or planes not being
	// specified to this list, FlightWindow::updateErrors should check if
	// this is a non-error (see there for an explanation).
	CHECK_FEHLER (FLUG, id_invalid (id), ff_keine_id)
	CHECK_FEHLER (FLUG, id_invalid (planeId), ff_kein_flugzeug)
	CHECK_FEHLER (FLUG, sa && sa->personRequired && id_invalid (pilotId) && pilotFirstName.isEmpty () && pilotLastName.isEmpty (), ff_kein_pilot)
	CHECK_FEHLER (FLUG, id_invalid (pilotId) && !pilotFirstName.isEmpty () && pilotLastName.isEmpty (), ff_pilot_nur_vorname);
	CHECK_FEHLER (FLUG, id_invalid (pilotId) && !pilotLastName.isEmpty () && pilotFirstName.isEmpty (), ff_pilot_nur_nachname);
	CHECK_FEHLER (FLUG, id_invalid (pilotId) && !pilotLastName.isEmpty () && !pilotFirstName.isEmpty (), ff_pilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && id_invalid (copilotId) && !copilotFirstName.isEmpty () && copilotLastName.isEmpty (), ff_begleiter_nur_vorname);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && id_invalid (copilotId) && !copilotLastName.isEmpty () && copilotFirstName.isEmpty (), ff_begleiter_nur_nachname);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && id_invalid (copilotId) && !copilotLastName.isEmpty () && !copilotFirstName.isEmpty (), ff_begleiter_nicht_identifiziert);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && pilotId!=0 && pilotId==copilotId, ff_pilot_gleich_begleiter)
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->isAirtow () && id_invalid (towpilotId) && !towpilotFirstName.isEmpty () && towpilotLastName.isEmpty (), ff_towpilot_nur_vorname);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->isAirtow () && id_invalid (towpilotId) && !towpilotLastName.isEmpty () && towpilotFirstName.isEmpty (), ff_towpilot_nur_nachname);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->isAirtow () && id_invalid (towpilotId) && !towpilotLastName.isEmpty () && !towpilotFirstName.isEmpty (), ff_towpilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->isAirtow () && towpilotId!=0 && pilotId==towpilotId, ff_pilot_gleich_towpilot)
	CHECK_FEHLER (FLUG, id_invalid (copilotId) && (type==typeTraining2) && copilotLastName.isEmpty () && copilotFirstName.isEmpty (), ff_schulung_ohne_begleiter)
	// TODO einsitzige Schulung mit Begleiter
	CHECK_FEHLER (FLUG, copilotId!=0 && !typeCopilotRecorded (type), ff_begleiter_nicht_erlaubt)
	CHECK_FEHLER (FLUG, departsHere () && landsHere () && landed && !departed, ff_nur_gelandet)
	CHECK_FEHLER (FLUG, departsHere () && landsHere () && departed && landed && departureTime>landingTime, ff_landung_vor_start)
	CHECK_FEHLER (FLUG, id_invalid (launchMethodId) && departed && departsHere (), ff_keine_startart)
	CHECK_FEHLER (FLUG, mode==modeNone, ff_kein_modus)
	CHECK_FEHLER (FLUG, type==typeNone, ff_kein_flugtyp)
	CHECK_FEHLER (FLUG, numLandings<0, ff_landungen_negativ)
	CHECK_FEHLER (FLUG, landsHere () && numLandings==0 && landed, ff_landungen_null)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && type==typeTraining2, ff_doppelsitzige_schulung_in_einsitzer)
	CHECK_FEHLER (FLUG, (departed || !departsHere ()) && eintrag_ist_leer (departureLocation), ff_kein_startort)
	CHECK_FEHLER (FLUG, (landed || !landsHere ()) && eintrag_ist_leer (landingLocation), ff_kein_zielort)
	CHECK_FEHLER (SCHLEPP, sa && sa->isAirtow() && (towflightLanded || !towflightLandsHere ()) && eintrag_ist_leer (towflightLandingLocation), ff_kein_zielort_sfz)
	CHECK_FEHLER (FLUG, fz && fz->category==Plane::categoryGlider && numLandings>1 && sa && !sa->isAirtow (), ff_segelflugzeug_landungen)
	CHECK_FEHLER (FLUG, fz && fz->category==Plane::categoryGlider && !landed && numLandings>0 && sa && !sa->isAirtow (), ff_segelflugzeug_landungen_ohne_landung)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && typeCopilotRecorded (type) && copilotId!=0, ff_begleiter_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && type==typeGuestPrivate, ff_gastflug_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && type==typeGuestExternal, ff_gastflug_in_einsitzer)
	//CHECK_FEHLER (FLUG, fz && fz->category==categoryGlider && sa && launchMethod==sa_ss, ff_segelflugzeug_selbststart)
	CHECK_FEHLER (FLUG, departsHere () && numLandings>0 && !departed, ff_landungen_ohne_start)
	CHECK_FEHLER (FLUG, departsHere ()!=landsHere () && departureLocation==landingLocation, ff_startort_gleich_zielort)
	CHECK_FEHLER (SCHLEPP, sa && sa->isAirtow () && !sa->towplaneKnown () && id_invalid (towplaneId), ff_kein_schleppflugzeug)
	CHECK_FEHLER (SCHLEPP, sa && sfz && sa->isAirtow () && !sa->towplaneKnown () && sfz->category==Plane::categoryGlider, ff_towplane_is_glider);

	return ff_ok;
#undef CHECK_FEHLER
#undef FLUG
#undef SCHLEPP
}


// ****************
// ** Formatting **
// ****************

QString personToString (db_id id, QString firstName, QString lastName)
{
	if (id_valid (id))
		return QString::number (id);
	else if (eintrag_ist_leer(firstName) && eintrag_ist_leer(lastName))
		return "-";
	else
		return QString ("(%1, %2)")
			.arg (eintrag_ist_leer (lastName)?QString ("?"):lastName)
			.arg (eintrag_ist_leer (firstName)?QString ("?"):firstName);
}

QString timeToString (bool performed, Time time)
{
	if (performed)
		return time.csv_string(tz_utc)+"Z";
	else
		return "-";
}

QString Flight::toString () const
{
	return QString ("id=%1, plane=%2, type=%3, pilot=%4, copilot=%5, mode=%6, "
		"launchMethod=%7, towplane=%8, towpilot=%9, towFlightMode=%10, "
		"launchTime=%11, landingTime=%12, towflightLandingTime=%13, "
		"departure='%14', destination='%15', towFlightDestination='%16', "
		"numLandings=%17, comment='%18', accountingNote='%19'")

		.arg (id)
		.arg (planeId)
		.arg (shortTypeText (type))
		.arg (personToString (pilotId, pilotFirstName, pilotLastName))
		.arg (personToString (copilotId, copilotFirstName, copilotLastName))
		.arg (modeText (mode))

		.arg (launchMethodId)
		.arg (towplaneId)
		.arg (personToString (towpilotId, towpilotFirstName, towpilotLastName))
		.arg (modeText (towflightMode))

		.arg (timeToString (departed, departureTime))
		.arg (timeToString (landed, landingTime))
		.arg (timeToString (towflightLanded, towflightLandingTime))

		.arg (departureLocation)
		.arg (landingLocation)
		.arg (towflightLandingLocation)

		.arg (numLandings)
		.arg (comments)
		.arg (accountingNotes)
		;
}


// **********
// ** Misc **
// **********

Flight Flight::makeTowflight (db_id theTowplaneId, db_id towLaunchMethod) const
{
	Flight towflight;

	// The tow flight gets the same ID because there would be no way to get
	// the ID for a given tow flight. The tow flight can be distinguished
	// from the real flight by the flight type (by calling isTowflight ()).
	towflight.id=id;

	// Always use the towplane ID passed by the caller, even if it is invalid -
	// this means that the towplane specified in the launch method was not found.
	towflight.planeId=theTowplaneId;

	// The towflight's pilot is our towpilot and there is not copilot and
	// towpilot
	towflight.pilotId=towpilotId;
	towflight.copilotId=invalid_id;
	towflight.towpilotId=invalid_id;

	towflight.departureTime=departureTime;							// The tow flight started the same time as the towed flight.
	towflight.landingTime=towflightLandingTime;			// The tow flight landing time is our landingTimeTowflight.
	towflight.towflightLandingTime=Time (); 			// The tow flight has no tow flight.

	// The launchMethod of the tow flight is given as a parameter.
	towflight.launchMethodId=towLaunchMethod;

	towflight.type=typeTow;
	towflight.departureLocation=departureLocation;							// The tow flight started the same place as the towed flight.
	towflight.landingLocation=towflightLandingLocation;							// The tow flight landing place is our destinationAirfieldTowplane.
	towflight.towflightLandingLocation="";

	towflight.numLandings=(towflightLandsHere () && towflightLanded)?1:0;

	towflight.comments=QString::fromUtf8 ("Schleppflug für Flug Nr. %1").arg (id);
	towflight.accountingNotes="";
	towflight.mode=towflightMode;
	towflight.towflightMode=modeNone;
	towflight.pilotFirstName=towpilotFirstName;
	towflight.pilotLastName=towpilotLastName;
	towflight.copilotFirstName="";
	towflight.copilotLastName="";
	towflight.towpilotFirstName="";
	towflight.towpilotLastName="";
	towflight.towplaneId=invalid_id;
	towflight.departed=departed;
	towflight.landed=towflightLanded;
	towflight.towflightLanded=false;

	return towflight;
}

// TODO move to PlaneLog
bool Flight::collectiveLogEntryPossible (const Flight *prev, const Plane *plane) const
{
	// Only allow if the previous flight and the current flight start and land
	// at the same place.
	if (prev->mode!=modeLocal || mode!=modeLocal) return false;
	if (prev->departureLocation!=prev->landingLocation) return false;
	if (prev->landingLocation!=departureLocation) return false;
	if (departureLocation!=landingLocation) return false;

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
		",pilot_first_name,pilot_last_name,copilot_first_name,copilot_last_name" // 4 Σ19
		",towflight_landing_time,towflight_mode,towflight_landing_location,towplane_id" // 4 Σ23
		",accounting_notes,comments" // 2 Σ25
		",towpilot_id,towpilot_first_name,towpilot_last_name" // 3 Σ28
		;
}

Flight Flight::createFromQuery (const QSqlQuery &q)
{
	Flight f (q.value (0).toLongLong ());

	f.pilotId          =q.value (1).toLongLong ();
	f.copilotId        =q.value (2).toLongLong ();
	f.planeId          =q.value (3).toLongLong ();
	f.type             =typeFromDb (
	                    q.value (4).toString   ());
	f.mode             =modeFromDb (
	                    q.value (5).toString   ());
	f.departed         =q.value (6).toBool     ();
	f.landed           =q.value (7).toBool     ();
	f.towflightLanded  =q.value (8).toBool     ();

	f.launchMethodId    =q.value ( 9).toLongLong ();
	f.departureLocation =q.value (10).toString   ();
	f.landingLocation   =q.value (11).toString   ();
	f.numLandings       =q.value (12).toInt      ();
	f.departureTime     =Time::create (
	                     q.value (13).toDateTime (), tz_utc);
	f.landingTime       =Time::create (
	                     q.value (14).toDateTime (), tz_utc);

	f.pilotFirstName   =q.value (15).toString ();
	f.pilotLastName    =q.value (16).toString ();
	f.copilotFirstName =q.value (17).toString ();
	f.copilotLastName  =q.value (18).toString ();

	f.towflightLandingTime     =Time::create (
	                            q.value (19).toDateTime (), tz_utc);
	f.towflightMode            =modeFromDb (
	                            q.value (20).toString   ());
	f.towflightLandingLocation =q.value (21).toString   ();
	f.towplaneId               =q.value (22).toLongLong ();

	f.accountingNotes =q.value (23).toString ();
	f.comments        =q.value (24).toString ();

	f.towpilotId         =q.value (25).toLongLong ();
	f.towpilotFirstName  =q.value (26).toString   ();
	f.towpilotLastName   =q.value (27).toString   ();

	return f;
}

QString Flight::insertValueList ()
{
	QString columnList=
		"pilot_id,copilot_id,plane_id,type,mode,departed,landed,towflight_landed" // 8
		",launch_method_id,departure_location,landing_location,num_landings,departure_time,landing_time" // 6 Σ14
		",pilot_first_name,pilot_last_name,copilot_first_name,copilot_last_name" // 4 Σ18
		",towflight_landing_time,towflight_mode,towflight_landing_location,towplane_id" // 4 Σ22
		",accounting_notes,comments" // 2 Σ24
		",towpilot_id,towpilot_first_name,towpilot_last_name" // 3 Σ27
		;

	QString placeholderList="?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?";

	return QString ("(%1) values (%2)").arg (columnList).arg (placeholderList);
}


QString Flight::updateValueList ()
{
	return
		"pilot_id=?,copilot_id=?,plane_id=?,type=?,mode=?,departed=?,landed=?,towflight_landed=?" // 8
		",launch_method_id=?,departure_location=?,landing_location=?,num_landings=?,departure_time=?,landing_time=?" // 6 Σ14
		",pilot_first_name=?,pilot_last_name=?,copilot_first_name=?,copilot_last_name=?" // 4 Σ18
		",towflight_landing_time=?,towflight_mode=?,towflight_landing_location=?,towplane_id=?" // 4 Σ22
		",accounting_notes=?,comments=?" // 2 Σ24
		",towpilot_id=?,towpilot_first_name=?,towpilot_last_name=?"; // 3 Σ27
		;
}

void Flight::bindValues (QSqlQuery &q) const
{
	q.addBindValue (pilotId);
	q.addBindValue (copilotId);
	q.addBindValue (planeId);
	q.addBindValue (typeToDb (type));
	q.addBindValue (modeToDb (mode));
	q.addBindValue (departed);
	q.addBindValue (landed);
	q.addBindValue (towflightLanded);

	q.addBindValue (launchMethodId);
	q.addBindValue (departureLocation);
	q.addBindValue (landingLocation);
	q.addBindValue (numLandings);
	q.addBindValue (departureTime.toUtcQDateTime ());
	q.addBindValue (landingTime.toUtcQDateTime ());

	q.addBindValue (pilotFirstName);
	q.addBindValue (pilotLastName);
	q.addBindValue (copilotFirstName);
	q.addBindValue (copilotLastName);

	q.addBindValue (towflightLandingTime.toUtcQDateTime ());
	q.addBindValue (modeToDb (towflightMode));
	q.addBindValue (towflightLandingLocation);
	q.addBindValue (towplaneId);

	q.addBindValue (accountingNotes);
	q.addBindValue (comments);

	q.addBindValue (towpilotId);
	q.addBindValue (towpilotFirstName);
	q.addBindValue (towpilotLastName);
}

QList<Flight> Flight::createListFromQuery (QSqlQuery &q)
{
	QList<Flight> list;

	while (q.next ())
		list.append (createFromQuery (q));

	return list;
}


// *** Enum mappers

QString Flight::modeToDb (Flight::Mode mode)
{
	switch (mode)
	{
		case modeLocal   : return "l";
		case modeComing  : return "k";
		case modeLeaving : return "g";
		case modeNone    : return "?";
		// no default
	}

	assert (false);
	return "?";
}

Flight::Mode Flight::modeFromDb (QString mode)
{
	if      (mode=="l") return modeLocal;
	else if (mode=="k") return modeComing;
	else if (mode=="g") return modeLeaving;
	else                return modeNone;
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
