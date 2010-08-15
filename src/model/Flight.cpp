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

Flight::Flight ()
{
	initialize (invalidId);
}

Flight::Flight (dbId id)
{
	initialize (id);
}

void Flight::initialize (dbId id)
{
	this->id=id;

	planeId         =invalidId;
	numLandings     =invalidId;
	pilotId         =invalidId;
	copilotId       =invalidId;
	towpilotId      =invalidId;
	launchMethodId  =invalidId;
	towplaneId      =invalidId;

	type          =typeNone;
	mode          =modeLocal;
	towflightMode =modeLocal;

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
	return idValid (pilotId) ||
		!isNone (pilotLastName, pilotFirstName);
}

bool Flight::copilotSpecified () const
{
	return idValid (copilotId) ||
		!isNone (copilotLastName, copilotFirstName);
}

bool Flight::towpilotSpecified () const
{
	return idValid (towpilotId) ||
		!isNone (towpilotLastName, towpilotFirstName);
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
		if (idInvalid (launchMethodId)) return false;

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (launchMethodId);
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
		if (idInvalid (launchMethodId)) return invalidId;

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (launchMethodId);
		if (!launchMethod.isAirtow ()) return invalidId;

		if (launchMethod.towplaneKnown ())
			return cache.getPlaneIdByRegistration (launchMethod.towplaneRegistration);
		else
			return towplaneId;
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
	notPossibleIf (landsHere () && landed, "Der Flug ist bereits gelandet");

	// Does not dpeart here
	notPossibleIf (!departsHere (), "Der Flug startet nicht hier.");

	// Already departed
	notPossibleIf (departed, "Der Flug ist bereits gestartet.");

	return true;
}

bool Flight::canLand (QString *reason) const
{
	// Already landed
	notPossibleIf (landed, "Der Flug ist bereits gelandet.");

	// Does not land here (only applies to non-towflights)
	notPossibleIf (!isTowflight () && !landsHere (), "Der Flug landet nicht hier.");

	// Must depart first
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

	// Must depart first
	notPossibleIf (departsHere () && !departed, "Der Flug ist noch nicht gestartet.");

	return true;
}

bool Flight::canTowflightLand (QString *reason) const
{
	// Already landed
	notPossibleIf (towflightLanded, "Der Schleppflug ist bereits gelandet.");

	// Must depart first
	notPossibleIf (departsHere () && !departed, "Der Flug ist noch nicht gestartet.");

	return true;
}

#undef notPossibleIf

bool Flight::departNow (bool force)
{
	if (force || canDepart ())
	{
		departureTime=nullSeconds (QDateTime::currentDateTime ().toUTC ());
		departed=true;
		return true;
	}

	return false;
}

bool Flight::landNow (bool force)
{
	if (force || canLand ())
	{
		landingTime=nullSeconds (QDateTime::currentDateTime ().toUTC ());
		numLandings++;
		landed=true;

		if (isNone (landingLocation))
			landingLocation=Settings::instance ().location;

		return true;
	}

	return false;
}

bool Flight::landTowflightNow (bool force)
{
	if (force || canTowflightLand ())
	{
		towflightLandingTime=nullSeconds (QDateTime::currentDateTime ().toUTC ());
		towflightLanded=true;
		if (towflightLandsHere () && isNone (towflightLandingLocation))
			towflightLandingLocation=Settings::instance ().location;
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

// TODO remove
QDate Flight::effdatum (Qt::TimeSpec spec) const
{
	// TODO this assumes that every flight at least departs or lands here.
	return effectiveTime ().toTimeSpec (spec).date ();
}

QDate Flight::getEffectiveDate (Qt::TimeSpec spec, QDate defaultDate) const
{
	// TODO this assumes that every flight at least departs or lands here.
	if (departsHere () && departed)
		return departureTime.toTimeSpec (spec).date ();

	if (landsHere () && landed)
		return landingTime.toTimeSpec (spec).date ();

	return defaultDate;
}

QDateTime Flight::effectiveTime () const
{
	// TODO this assumes that every flight at least departs or lands here.
	if (departsHere () && departed) return departureTime;
	if (landsHere () && landed) return landingTime;
	return QDateTime ();
}

QTime Flight::flightDuration () const
{
	if (departed && landed)
		return QTime ().addSecs (departureTime.secsTo (landingTime));
	else if (departed)
		return QTime ().addSecs (departureTime.secsTo (QDateTime::currentDateTime ().toUTC ()));
	else
		return QTime ();
}

QTime Flight::towflightDuration () const
{
	if (departed && towflightLanded)
		return QTime ().addSecs (departureTime.secsTo (towflightLandingTime));
	else if (departed)
		return QTime ().addSecs (departureTime.secsTo (QDateTime::currentDateTime ().toUTC ()));
	else
		return QTime ();
}


// ********************
// ** Error checking **
// ********************

/**
 * This uses fehlerhaft, does not consider schlepp_fehlerhaft
 *
 * @param cache
 * @return
 */
bool Flight::isErroneous (Cache &cache) const
{
	Plane *thePlane=cache.getNewObject<Plane> (planeId);
	LaunchMethod *theLaunchMethod=cache.getNewObject<LaunchMethod> (launchMethodId);

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
		// TODO tr for all; .arg
		case ff_ok: return utf8 ("Kein Fehler");
		case ff_keine_id: return utf8 ("Flug hat keine ID");
		case ff_kein_flugzeug: return utf8 ("Kein Flugzeug angegeben");
		// TODO use person_bezeichnung (flightType) (oder wie die heißt) here
		case ff_pilot_nur_nachname: return utf8 ("Für den %1 ist nur ein Nachname angegeben").arg (utf8 (type==typeTraining2?"Flugschüler":"Piloten"));
		case ff_pilot_nur_vorname: return  utf8 ("Für den ")+utf8 (type==typeTraining2?"Flugschüler":"Piloten")+" ist nur ein Vorname angegeben";
		case ff_pilot_nicht_identifiziert: return  "Der "+utf8 (type==typeTraining2?"Flugschüler":"Pilot")+" ist nicht identifiziert";
		case ff_begleiter_nur_nachname: return utf8 ("Für den %1 ist nur ein Nachname angegeben").arg (QString (type==typeTraining2?"Fluglehrer":"Begleiter"));
		case ff_begleiter_nur_vorname: return  utf8 ("Für den ")+QString (type==typeTraining2?"Fluglehrer":"Begleiter")+" ist nur ein Vorname angegeben";
		case ff_begleiter_nicht_identifiziert: return  "Der "+QString (type==typeTraining2?"Fluglehrer":"Begleiter")+" ist nicht identifiziert";
		case ff_towpilot_nur_nachname: return utf8 ("Für den Schleppiloten ist nur ein Nachname angegeben");
		case ff_towpilot_nur_vorname: return  utf8 ("Für den Schleppiloten ist nur ein Vorname angegeben");
		case ff_towpilot_nicht_identifiziert: return  "Der Schleppilot ist nicht identifiziert";
		case ff_kein_pilot: return utf8 ("Kein %1 angegeben").arg (utf8 (type==typeTraining2 || type==typeTraining1?"Flugschüler":"Pilot"));
		case ff_pilot_gleich_begleiter: return utf8 (type==typeTraining2?"Flugschüler und Fluglehrer":"Pilot und Begleiter")+" sind identisch";
		case ff_pilot_gleich_towpilot: return utf8 (type==typeTraining2?"Flugschüler":"Pilot")+" und Schlepppilot sind identisch";
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
	bool recordTowpilot=Settings::instance ().recordTowpilot;

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
	CHECK_FEHLER (FLUG, idInvalid (id), ff_keine_id)
	CHECK_FEHLER (FLUG, idInvalid (planeId), ff_kein_flugzeug)
	CHECK_FEHLER (FLUG, sa && sa->personRequired && idInvalid (pilotId) && pilotLastName.isEmpty () && pilotFirstName.isEmpty (), ff_kein_pilot)
	CHECK_FEHLER (FLUG, idInvalid (pilotId) && !pilotLastName.isEmpty () && pilotFirstName.isEmpty (), ff_pilot_nur_nachname);
	CHECK_FEHLER (FLUG, idInvalid (pilotId) && !pilotLastName.isEmpty () && pilotFirstName.isEmpty (), ff_pilot_nur_vorname);
	CHECK_FEHLER (FLUG, idInvalid (pilotId) && !pilotLastName.isEmpty () && !pilotFirstName.isEmpty (), ff_pilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && idInvalid (copilotId) && !copilotLastName.isEmpty () && copilotFirstName.isEmpty (), ff_begleiter_nur_nachname);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && idInvalid (copilotId) && !copilotLastName.isEmpty () && copilotFirstName.isEmpty (), ff_begleiter_nur_vorname);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && idInvalid (copilotId) && !copilotLastName.isEmpty () && !copilotFirstName.isEmpty (), ff_begleiter_nicht_identifiziert);
	CHECK_FEHLER (FLUG, typeCopilotRecorded (type) && pilotId!=0 && pilotId==copilotId, ff_pilot_gleich_begleiter)
	CHECK_FEHLER (FLUG, recordTowpilot && sa && sa->isAirtow () && idInvalid (towpilotId) && !towpilotLastName.isEmpty () && towpilotFirstName.isEmpty (), ff_towpilot_nur_nachname);
	CHECK_FEHLER (FLUG, recordTowpilot && sa && sa->isAirtow () && idInvalid (towpilotId) && !towpilotLastName.isEmpty () && towpilotFirstName.isEmpty (), ff_towpilot_nur_vorname);
	CHECK_FEHLER (FLUG, recordTowpilot && sa && sa->isAirtow () && idInvalid (towpilotId) && !towpilotLastName.isEmpty () && !towpilotFirstName.isEmpty (), ff_towpilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, recordTowpilot && sa && sa->isAirtow () && towpilotId!=0 && pilotId==towpilotId, ff_pilot_gleich_towpilot)
	CHECK_FEHLER (FLUG, idInvalid (copilotId) && (type==typeTraining2) && copilotLastName.isEmpty () && copilotFirstName.isEmpty (), ff_schulung_ohne_begleiter)
	// TODO einsitzige Schulung mit Begleiter
	CHECK_FEHLER (FLUG, copilotId!=0 && !typeCopilotRecorded (type), ff_begleiter_nicht_erlaubt)
	CHECK_FEHLER (FLUG, departsHere () && landsHere () && landed && !departed, ff_nur_gelandet)
	CHECK_FEHLER (FLUG, departsHere () && landsHere () && departed && landed && departureTime>landingTime, ff_landung_vor_start)
	CHECK_FEHLER (FLUG, idInvalid (launchMethodId) && departsHere () && departed && !isTowflight (), ff_keine_startart)
	CHECK_FEHLER (FLUG, type==typeNone, ff_kein_flugtyp)
	CHECK_FEHLER (FLUG, numLandings<0, ff_landungen_negativ)
	CHECK_FEHLER (FLUG, landsHere () && numLandings==0 && landed, ff_landungen_null)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && type==typeTraining2, ff_doppelsitzige_schulung_in_einsitzer)
	CHECK_FEHLER (FLUG, (departed || !departsHere ()) && isNone (departureLocation), ff_kein_startort)
	CHECK_FEHLER (FLUG, (landed || !landsHere ()) && isNone (landingLocation), ff_kein_zielort)
	CHECK_FEHLER (SCHLEPP, sa && sa->isAirtow() && (towflightLanded || !towflightLandsHere ()) && isNone (towflightLandingLocation), ff_kein_zielort_sfz)
	CHECK_FEHLER (FLUG, fz && fz->category==Plane::categoryGlider && numLandings>1 && sa && !sa->isAirtow (), ff_segelflugzeug_landungen)
	CHECK_FEHLER (FLUG, fz && fz->category==Plane::categoryGlider && !landed && numLandings>0 && sa && !sa->isAirtow (), ff_segelflugzeug_landungen_ohne_landung)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && typeCopilotRecorded (type) && copilotId!=0, ff_begleiter_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && type==typeGuestPrivate, ff_gastflug_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && type==typeGuestExternal, ff_gastflug_in_einsitzer)
	//CHECK_FEHLER (FLUG, fz && fz->category==categoryGlider && sa && launchMethod==sa_ss, ff_segelflugzeug_selbststart)
	CHECK_FEHLER (FLUG, departsHere () && numLandings>0 && !departed, ff_landungen_ohne_start)
	CHECK_FEHLER (FLUG, departsHere ()!=landsHere () && departureLocation==landingLocation, ff_startort_gleich_zielort)
	CHECK_FEHLER (SCHLEPP, sa && sa->isAirtow () && !sa->towplaneKnown () && idInvalid (towplaneId), ff_kein_schleppflugzeug)
	CHECK_FEHLER (SCHLEPP, sa && sfz && sa->isAirtow () && !sa->towplaneKnown () && sfz->category==Plane::categoryGlider, ff_towplane_is_glider);

	return ff_ok;
#undef CHECK_FEHLER
#undef FLUG
#undef SCHLEPP
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

		.arg (id)
		.arg (planeId)
		.arg (shortTypeText (type))
		.arg (personToString (pilotId, pilotLastName, pilotFirstName))
		.arg (personToString (copilotId, copilotLastName, copilotFirstName))
		.arg (modeText (mode))

		.arg (launchMethodId)
		.arg (towplaneId)
		.arg (personToString (towpilotId, towpilotLastName, towpilotFirstName))
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
	towflight.id=id;

	// Always use the towplane ID passed by the caller, even if it is invalid -
	// this means that the towplane specified in the launch method was not found.
	towflight.planeId=theTowplaneId;

	// The towflight's pilot is our towpilot and there is not copilot and
	// towpilot
	towflight.pilotId=towpilotId;
	towflight.copilotId=invalidId;
	towflight.towpilotId=invalidId;

	towflight.departureTime=departureTime;              // The tow flight departed the same time as the towed flight.
	towflight.landingTime=towflightLandingTime;         // The tow flight landing time is our landingTimeTowflight.
	towflight.towflightLandingTime=QDateTime ();        // The tow flight has no tow flight.

	// The launchMethod of the tow flight is given as a parameter.
	towflight.launchMethodId=towLaunchMethod;

	towflight.type=typeTow;
	towflight.departureLocation=departureLocation;      // The tow flight departed the same location as the towed flight.
	towflight.landingLocation=towflightLandingLocation; // The tow flight landing place is our landingLocationTowplane.
	towflight.towflightLandingLocation="";

	towflight.numLandings=(towflightLandsHere () && towflightLanded)?1:0;

	towflight.comments=utf8 ("Schleppflug für Flug Nr. %1").arg (id);
	towflight.accountingNotes="";
	towflight.mode=towflightMode;
	towflight.towflightMode=modeLocal;
	towflight.pilotLastName=towpilotLastName;
	towflight.pilotFirstName=towpilotFirstName;
	towflight.copilotLastName="";
	towflight.copilotFirstName="";
	towflight.towpilotLastName="";
	towflight.towpilotFirstName="";
	towflight.towplaneId=invalidId;
	towflight.departed=departed;
	towflight.landed=towflightLanded;
	towflight.towflightLanded=false;

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
	if (prev->mode!=modeLocal || mode!=modeLocal) return false;
	if (prev->departureLocation.trimmed ().toLower ()!=prev->landingLocation.trimmed ().toLower ()) return false;
	if (prev->  landingLocation.trimmed ().toLower ()!=    departureLocation.trimmed ().toLower ()) return false;
	if (      departureLocation.trimmed ().toLower ()!=      landingLocation.trimmed ().toLower ()) return false;

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

	f.pilotId          =result.value (1).toLongLong ();
	f.copilotId        =result.value (2).toLongLong ();
	f.planeId          =result.value (3).toLongLong ();
	f.type             =typeFromDb (
	                    result.value (4).toString   ());
	f.mode             =modeFromDb (
	                    result.value (5).toString   ());
	f.departed         =result.value (6).toBool     ();
	f.landed           =result.value (7).toBool     ();
	f.towflightLanded  =result.value (8).toBool     ();

	f.launchMethodId    =result.value ( 9).toLongLong ();
	f.departureLocation =result.value (10).toString   ();
	f.landingLocation   =result.value (11).toString   ();
	f.numLandings       =result.value (12).toInt      ();
	f.departureTime     =result.value (13).toDateTime (); f.departureTime.setTimeSpec (Qt::UTC); // not toUTC
	f.landingTime       =result.value (14).toDateTime (); f.landingTime.setTimeSpec (Qt::UTC); // not toUTC

	f.pilotLastName    =result.value (15).toString ();
	f.pilotFirstName   =result.value (16).toString ();
	f.copilotLastName  =result.value (17).toString ();
	f.copilotFirstName =result.value (18).toString ();

	f.towflightLandingTime     =result.value (19).toDateTime (); f.towflightLandingTime.setTimeSpec (Qt::UTC); // not toUTC
	f.towflightMode            =modeFromDb (
	                            result.value (20).toString   ());
	f.towflightLandingLocation =result.value (21).toString   ();
	f.towplaneId               =result.value (22).toLongLong ();

	f.accountingNotes =result.value (23).toString ();
	f.comments        =result.value (24).toString ();

	f.towpilotId         =result.value (25).toLongLong ();
	f.towpilotLastName   =result.value (26).toString   ();
	f.towpilotFirstName  =result.value (27).toString   ();

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
	q.bind (pilotId);
	q.bind (copilotId);
	q.bind (planeId);
	q.bind (typeToDb (type));
	q.bind (modeToDb (mode));
	q.bind (departed);
	q.bind (landed);
	q.bind (towflightLanded);

	q.bind (launchMethodId);
	q.bind (departureLocation);
	q.bind (landingLocation);
	q.bind (numLandings);
	q.bind (departureTime.toUTC ());
	q.bind (landingTime.toUTC ());

	q.bind (pilotLastName);
	q.bind (pilotFirstName);
	q.bind (copilotLastName);
	q.bind (copilotFirstName);

	q.bind (towflightLandingTime.toUTC ());
	q.bind (modeToDb (towflightMode));
	q.bind (towflightLandingLocation);
	q.bind (towplaneId);

	q.bind (accountingNotes);
	q.bind (comments);

	q.bind (towpilotId);
	q.bind (towpilotLastName);
	q.bind (towpilotFirstName);
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
	Plane *plane=cache.getNewObject<Plane> (planeId);
	LaunchMethod *launchMethod=cache.getNewObject<LaunchMethod> (launchMethodId);

	bool error=fehlerhaft (plane, NULL, launchMethod);

	delete plane;
	delete launchMethod;

	return flightColor (mode, error, isTowflight (), departed, landed);
}
