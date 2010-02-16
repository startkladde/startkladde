#include "Flight.h"

#include <iostream>

#include "src/config/Options.h"

#include "src/db/DataStorage.h"


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

/**
  * Constructs an Flight instance with empty values.
  */
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

	plane         =invalid_id;
	numLandings   =invalid_id;
	pilot         =invalid_id;
	copilot       =invalid_id;
	towpilot      =invalid_id;
	launchMethod  =invalid_id;
	towplane      =invalid_id;

	flightType=ftNone;
	mode          =fmNone;
	modeTowflight =fmNone;

	started         =false;
	landed          =false;
	towflightLanded =false;
}

/**
 * Deprecated in favor of isFlying
 *
 * @return
 */
bool Flight::fliegt () const
	/*
	 * Determines whether a flight has landed.
	 * Return value:
	 *   - true if the flight is flying
	 *   - false else.
	 */
{
	return happened () && !finished ();
}

bool Flight::sfz_fliegt () const
	/*
	 * Determine if the towplane is flying.
	 * Return value:
	 *   - true if the towplane is flying.
	 *   - false else.
	 */
{
	return happened () && !towflightLanded;
}

bool Flight::vorbereitet () const
	/*
	 * Check if the flight is prepared.
	 * Return value:
	 *   - true if the flight is prepared.
	 *   - false else.
	 */
{
	return !happened ();
}

/**
 * Whether the flight either departed or landed; it need not have finished.
 * @return
 */
// TODO: guarantee that any flight for which this is not true has an effectiveDate
bool Flight::happened () const
{
	if (starts_here (mode) && started) return true;
	if (lands_here (mode) && landed) return true;
	return false;
}

bool Flight::finished () const
{
	if (isTowflight ())
		// For leaving towflights, landed means ended.
		return landed;
	else
		return (lands_here (mode)?landed:started);
}

Time Flight::flightDuration () const
	/*
	 * Calculate the flight time of the flight.
	 * Return value:
	 *   - the flight time.
	 */
{
	Time t;
	if (started && landed)
		t.set_to (launchTime.secs_to (&landingTime));
	else
	{
		Time now;
		now.set_current(true);
		t.set_to (launchTime.secs_to (&now));
	}

	return t;
}

Time Flight::towflightDuration () const
	/*
	 * Calculate the flight time of the towflight
	 * Return value:
	 *   - the flight time.
	 */
{
	Time t;
	if (started && towflightLanded)
		t.set_to (launchTime.secs_to (&landingTimeTowflight));
	else
	{
		Time now;
		now.set_current(true);
		t.set_to (launchTime.secs_to (&now));
	}

	return t;
}

bool Flight::fehlerhaft (Plane *fz, Plane *sfz, LaunchMethod *sa, QString *errorText) const
	/*
	 * Finds out if the flight contains an error.
	 * Parameters:
	 *   - fz: the plane data structure for the flight.
	 * Return value:
	 *   - true if any error was found
	 *   - false else.
	 */
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
	/*
	 * Makes a text describing a flight error.
	 * Parameters:
	 *   code: an error code.
	 * Return value:
	 *   the description.
	 */
{
	switch (code)
	{
		// TODO Utf8 for all
		case ff_ok: return QString::fromUtf8 ("Kein Fehler");
		case ff_keine_id: return QString::fromUtf8 ("Flug hat keine ID");
		case ff_kein_flugzeug: return QString::fromUtf8 ("Kein Flugzeug angegeben");
		// TODO use person_bezeichnung (flightType) (oder wie die heißt) here
		case ff_pilot_nur_nachname: return QString::fromUtf8 ("Für den "+QString (flightType==ftTraining2?"Flugschüler":"Piloten")+" ist nur ein Nachname angegeben");
		case ff_pilot_nur_vorname: return  "Für den "+QString (flightType==ftTraining2?"Flugschüler":"Piloten")+" ist nur ein Vorname angegeben";
		case ff_pilot_nicht_identifiziert: return  "Der "+QString (flightType==ftTraining2?"Flugschüler":"Pilot")+" ist nicht identifiziert";
		case ff_begleiter_nur_nachname: return QString::fromUtf8 ("Für den "+QString (flightType==ftTraining2?"Fluglehrer":"Begleiter")+" ist nur ein Nachname angegeben");
		case ff_begleiter_nur_vorname: return  "Für den "+QString (flightType==ftTraining2?"Fluglehrer":"Begleiter")+" ist nur ein Vorname angegeben";
		case ff_begleiter_nicht_identifiziert: return  "Der "+QString (flightType==ftTraining2?"Fluglehrer":"Begleiter")+" ist nicht identifiziert";
		case ff_towpilot_nur_nachname: return QString::fromUtf8 ("Für den Schleppiloten ist nur ein Nachname angegeben");
		case ff_towpilot_nur_vorname: return  "Für den Schleppiloten ist nur ein Vorname angegeben";
		case ff_towpilot_nicht_identifiziert: return  "Der Schleppilot ist nicht identifiziert";
		case ff_kein_pilot: return QString::fromUtf8 ("Kein "+QString (flightType==ftTraining2 || flightType==ftTraining1?"Flugschüler":"Pilot")+" angegeben");
		case ff_pilot_gleich_begleiter: return QString (flightType==ftTraining2?"Flugschüler und Fluglehrer":"Pilot und Begleiter")+" sind identisch";
		case ff_pilot_gleich_towpilot: return QString (flightType==ftTraining2?"Flugschüler":"Pilot")+" und Schlepppilot sind identisch";
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

FlightError Flight::errorCheck (int *index, bool check_flug, bool check_schlepp, Plane *fz, Plane *sfz, LaunchMethod *sa) const
	/*
	 * Does the unified error checking.
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
	 * Parameters:
	 *   - index: the index of the first error to check.
	 *   - check_flug: wheter to check the actual flight
	 *   - check_schlepp: whether to check the tow flight
	 * Return value:
	 *   - error code, or ff_ok, if no more error is found.
	 */
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
	CHECK_FEHLER (FLUG, id_invalid (plane), ff_kein_flugzeug)
	CHECK_FEHLER (FLUG, sa && sa->personRequired && id_invalid (pilot) && pvn.isEmpty () && pnn.isEmpty (), ff_kein_pilot)
	CHECK_FEHLER (FLUG, id_invalid (pilot) && !pvn.isEmpty () && pnn.isEmpty (), ff_pilot_nur_vorname);
	CHECK_FEHLER (FLUG, id_invalid (pilot) && !pnn.isEmpty () && pvn.isEmpty (), ff_pilot_nur_nachname);
	CHECK_FEHLER (FLUG, id_invalid (pilot) && !pnn.isEmpty () && !pvn.isEmpty (), ff_pilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, flightTypeCopilotRecorded (flightType) && id_invalid (copilot) && !bvn.isEmpty () && bnn.isEmpty (), ff_begleiter_nur_vorname);
	CHECK_FEHLER (FLUG, flightTypeCopilotRecorded (flightType) && id_invalid (copilot) && !bnn.isEmpty () && bvn.isEmpty (), ff_begleiter_nur_nachname);
	CHECK_FEHLER (FLUG, flightTypeCopilotRecorded (flightType) && id_invalid (copilot) && !bnn.isEmpty () && !bvn.isEmpty (), ff_begleiter_nicht_identifiziert);
	CHECK_FEHLER (FLUG, flightTypeCopilotRecorded (flightType) && pilot!=0 && pilot==copilot, ff_pilot_gleich_begleiter)
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && id_invalid (towpilot) && !tpvn.isEmpty () && tpnn.isEmpty (), ff_towpilot_nur_vorname);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && id_invalid (towpilot) && !tpnn.isEmpty () && tpvn.isEmpty (), ff_towpilot_nur_nachname);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && id_invalid (towpilot) && !tpnn.isEmpty () && !tpvn.isEmpty (), ff_towpilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && towpilot!=0 && pilot==towpilot, ff_pilot_gleich_towpilot)
	CHECK_FEHLER (FLUG, id_invalid (copilot) && (flightType==ftTraining2) && bnn.isEmpty () && bvn.isEmpty (), ff_schulung_ohne_begleiter)
	// TODO einsitzige Schulung mit Begleiter
	CHECK_FEHLER (FLUG, copilot!=0 && !flightTypeCopilotRecorded (flightType), ff_begleiter_nicht_erlaubt)
	CHECK_FEHLER (FLUG, starts_here (mode) && lands_here (mode) && landed && !started, ff_nur_gelandet)
	CHECK_FEHLER (FLUG, starts_here (mode) && lands_here (mode) && started && landed && launchTime>landingTime, ff_landung_vor_start)
	CHECK_FEHLER (FLUG, id_invalid (launchMethod) && started && starts_here (mode), ff_keine_startart)
	CHECK_FEHLER (FLUG, mode==fmNone, ff_kein_modus)
	CHECK_FEHLER (FLUG, flightType==ftNone, ff_kein_flugtyp)
	CHECK_FEHLER (FLUG, numLandings<0, ff_landungen_negativ)
	CHECK_FEHLER (FLUG, lands_here (mode) && numLandings==0 && landed, ff_landungen_null)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && flightType==ftTraining2, ff_doppelsitzige_schulung_in_einsitzer)
	CHECK_FEHLER (FLUG, (started || !starts_here (mode)) && eintrag_ist_leer (departureAirfield), ff_kein_startort)
	CHECK_FEHLER (FLUG, (landed || !lands_here (mode)) && eintrag_ist_leer (destinationAirfield), ff_kein_zielort)
	CHECK_FEHLER (SCHLEPP, sa && sa->is_airtow() && (towflightLanded || !lands_here (modeTowflight)) && eintrag_ist_leer (destinationAirfieldTowplane), ff_kein_zielort_sfz)
	CHECK_FEHLER (FLUG, fz && fz->category==Plane::categoryGlider && numLandings>1 && sa && !sa->is_airtow (), ff_segelflugzeug_landungen)
	CHECK_FEHLER (FLUG, fz && fz->category==Plane::categoryGlider && !landed && numLandings>0 && sa && !sa->is_airtow (), ff_segelflugzeug_landungen_ohne_landung)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && flightTypeCopilotRecorded (flightType) && copilot!=0, ff_begleiter_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && flightType==ftGuestPrivate, ff_gastflug_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->numSeats<=1 && flightType==ftGuestExternal, ff_gastflug_in_einsitzer)
	//CHECK_FEHLER (FLUG, fz && fz->category==categoryGlider && sa && launchMethod==sa_ss, ff_segelflugzeug_selbststart)
	CHECK_FEHLER (FLUG, starts_here (mode) && numLandings>0 && !started, ff_landungen_ohne_start)
	CHECK_FEHLER (FLUG, starts_here (mode)!=lands_here (mode) && departureAirfield==destinationAirfield, ff_startort_gleich_zielort)
	CHECK_FEHLER (SCHLEPP, sa && sa->is_airtow () && !sa->towplane_known () && id_invalid (towplane), ff_kein_schleppflugzeug)
	CHECK_FEHLER (SCHLEPP, sa && sfz && sa->is_airtow () && !sa->towplane_known () && sfz->category==Plane::categoryGlider, ff_towplane_is_glider);

	return ff_ok;
#undef CHECK_FEHLER
#undef FLUG
#undef SCHLEPP
}


bool Flight::startNow (bool force)
{
	if (force || canStart ())
	{
		launchTime.set_current (true);
		started=true;
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

		if (eintrag_ist_leer (destinationAirfield))
			destinationAirfield=opts.ort;

		return true;
	}

	return false;
}

bool Flight::landTowflightNow (bool force)
{
	if (force || canTowflightLand ())
	{
		landingTimeTowflight.set_current (true);
		towflightLanded=true;
		if (lands_here (modeTowflight) && eintrag_ist_leer (destinationAirfieldTowplane)) destinationAirfieldTowplane=opts.ort;
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



// TODO remove
QString Flight::typeString (lengthSpecification lenspec) const
	/*
	 * Generates a QString describing the type of the flight.
	 * Parameters:
	 *   - lenspec: the length to generate.
	 * Return value:
	 *   - the description.
	 */
{
	return flightTypeText (flightType, lenspec);
}



Time Flight::effectiveTime () const
{
	// TODO this assumes that every flight at least starts or lands here.
	if (starts_here (mode) && started) return launchTime;
	if (lands_here (mode) && landed) return landingTime;
	return Time ();
}



QString Flight::pilotDescription () const
	/*
	 * Return a description for the pilot.
	 * Return value:
	 *   - the description.
	 */
{
	return t_pilot_bezeichnung (flightType);
}

QString Flight::copilotDescription () const
	/*
	 * Return a description for the copilot.
	 * Return value:
	 *   - the description.
	 */
{
	return t_begleiter_bezeichnung (flightType);
}


QString Flight::incompletePilotName () const
	/*
	 * Makes the incomplete name of the pilot.
	 * Return value:
	 *   - the name.
	 */
{
	return incompletePersonName (pnn, pvn);
}

QString Flight::incompleteCopilotName () const
	/*
	 * Makes the incomplete name of the copilot.
	 * Return value:
	 *   - the name.
	 */
{
	return incompletePersonName (bnn, bvn);
}

QString Flight::incompleteTowpilotName () const
	/*
	 * Makes the incomplete name of the towpilot.
	 * Return value:
	 *   - the name.
	 */
{
	return incompletePersonName (tpnn, tpvn);
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



bool Flight::flight_lands_here () const
{
	return lands_here (mode);
}

bool Flight::flight_starts_here () const
{
	return starts_here (mode);
}


Flight Flight::makeTowflight (db_id towplaneId, db_id towLaunchMethod) const
{
	Flight towflight;

	// The tow flight gets the same ID because there would be no way to get
	// the ID for a given tow flight. The tow flight can be distinguished
	// from the real flight by the flight type (by calling isTowflight ()).
	towflight.id=id;

	// Always use the towplane ID passed by the caller, even if it is invalid -
	// this means that the towplane specified in the launch method was not found.
	towflight.plane=towplaneId;

	// The towflight's pilot is our towpilot and there is not copilot and
	// towpilot
	towflight.pilot=towpilot;
	towflight.copilot=invalid_id;
	towflight.towpilot=invalid_id;

	towflight.launchTime=launchTime;							// The tow flight started the same time as the towed flight.
	towflight.landingTime=landingTimeTowflight;			// The tow flight landing time is our landingTimeTowflight.
	towflight.landingTimeTowflight=Time (); 			// The tow flight has no tow flight.

	// The launchMethod of the tow flight is given as a parameter.
	towflight.launchMethod=towLaunchMethod;

	towflight.flightType=ftTow;
	towflight.departureAirfield=departureAirfield;							// The tow flight started the same place as the towed flight.
	towflight.destinationAirfield=destinationAirfieldTowplane;							// The tow flight landing place is our destinationAirfieldTowplane.
	towflight.destinationAirfieldTowplane="";

	towflight.numLandings=(towflightLandsHere () && towflightLanded)?1:0;

	towflight.comments=QString::fromUtf8 ("Schleppflug für Flug Nr. %1").arg (id);
	towflight.accountingNote="";
	towflight.mode=modeTowflight;
	towflight.modeTowflight=fmNone;
	towflight.pvn=tpvn;
	towflight.pnn=tpnn;
	towflight.bvn="";
	towflight.bnn="";
	towflight.tpvn="";
	towflight.tpnn="";
	towflight.towplane=invalid_id;
	towflight.started=started;
	towflight.landed=towflightLanded;
	towflight.towflightLanded=false;

	return towflight;
}

void Flight::get_towflight (Flight *towflight, db_id towplaneId, db_id towLaunchMethod) const
{
	*towflight=makeTowflight (towplaneId, towLaunchMethod);
}

// TODO move to PlaneLog
bool Flight::collective_bb_entry_possible (const Flight *prev, const Plane *plane) const
{
	// Only allow if the previous flight and the current flight start and land
	// at the same place.
	if (prev->mode!=fmLocal || mode!=fmLocal) return false;
	if (prev->departureAirfield!=prev->destinationAirfield) return false;
	if (prev->destinationAirfield!=departureAirfield) return false;
	if (departureAirfield!=destinationAirfield) return false;

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
		.arg (plane)
		.arg (flightTypeText (flightType, lsTable))
		.arg (personToString (pilot, pvn, pnn))
		.arg (personToString (copilot, bvn, bnn))
		.arg (flightModeText (mode, lsTable))

		.arg (launchMethod)
		.arg (towplane)
		.arg (personToString (towpilot, tpvn, tpnn))
		.arg (flightModeText (modeTowflight, lsTable))

		.arg (timeToString (started, launchTime))
		.arg (timeToString (landed, landingTime))
		.arg (timeToString (towflightLanded, landingTimeTowflight))

		.arg (departureAirfield)
		.arg (destinationAirfield)
		.arg (destinationAirfieldTowplane)

		.arg (numLandings)
		.arg (comments)
		.arg (accountingNote)
		;
}

bool Flight::operator< (const Flight &o) const
{
	return sort (&o)<0;
}

int Flight::sort (const Flight *other) const
	/*
	 * Return value:
	 *   - >0 if this flight is later
	 *   - <0 if this flight is earlier
	 *   - =0 if the flights are equal
	 */
{
	// Both prepared
	if (vorbereitet () && other->vorbereitet  ())
	{
		// Incoming prepared before local launching prepared
		if (starts_here (mode) && !starts_here (other->mode)) return 1;
		if (!starts_here (mode) && starts_here (other->mode)) return -1;

		// Flights are equal
		return 0;
	}

	// Prepared flights to the end
	if (vorbereitet ()) return 1;
	if (other->vorbereitet ()) return -1;

	// Sort by effective time
	Time t1=effectiveTime ();
	Time t2=other->effectiveTime ();
	if (t1>t2) return 1;
	if (t1<t2) return -1;
	return 0;
}



QDate Flight::effdatum (time_zone tz) const
        /*
         * Calculates the effective date, that is the date to use when sorting the
         * Table.
         * Parameters:
         *   - tz: the time zone to use.
         * Return value:
         *   - the effective date.
         */
{
        return effectiveTime ().get_qdate (tz);
}

QDate Flight::getEffectiveDate (time_zone tz, QDate defaultDate) const
{
	// TODO this assumes that every flight at least starts or lands here.
	if (starts_here (mode) && started)
		return launchTime.get_qdate (tz);

	if (lands_here (mode) && landed)
		return landingTime.get_qdate (tz);

	return defaultDate;
}


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

#define notPossibleIf(condition, reasonText) do { if (condition) { if (reason) *reason=reasonText; return false; } } while (0)

bool Flight::canStart (QString *reason) const
{
	// TODO only for flights of today

	// Already landed
	notPossibleIf (landsHere () && landed, "Der Flug ist bereits gelandet");

	// Does not start here
	notPossibleIf (!startsHere (), "Der Flug startet nicht hier.");

	// Already started
	notPossibleIf (started, "Der Flug ist bereits gestartet.");

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
	notPossibleIf (startsHere () && !started, "Der Flug ist noch nicht gestartet.");

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
	notPossibleIf (startsHere () && !started, "Der Flug ist noch nicht gestartet.");

	return true;
}

bool Flight::canTowflightLand (QString *reason) const
{
	// Already landed
	notPossibleIf (towflightLanded, "Der Schleppflug ist bereits gelandet.");

	// Must start first
	notPossibleIf (startsHere () && !started, "Der Flug ist noch nicht gestartet.");

	return true;
}

#undef notPossibleIf

/**
 * This uses fehlerhaft, does not consider schlepp_fehlerhaft
 *
 * @param dataStorage
 * @return
 */
bool Flight::isErroneous (DataStorage &dataStorage) const
{
	Plane *thePlane=dataStorage.getNewObject<Plane> (plane);
	LaunchMethod *theLaunchMethod=dataStorage.getNewObject<LaunchMethod> (launchMethod);

	bool erroneous=fehlerhaft (thePlane, NULL, theLaunchMethod);

	delete thePlane;
	delete theLaunchMethod;

	return erroneous;
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
		"id,pilot_id,copilot_id,plane_id,type,mode,status" // 7
		",launch_method_id,departure_location,landing_location,num_landings,departure_time,landing_time" // 6 Σ13
		",pilot_first_name,pilot_last_name,copilot_first_name,copilot_last_name" // 4 Σ17
		",towflight_landing_time,towflight_mode,towflight_landing_location,towplane_id" // 4 Σ21
		",accounting_notes,comments" // 2 Σ23
		",towpilot_id,towpilot_first_name,towpilot_last_name" // 3 Σ26
		;
}

Flight Flight::createFromQuery (const QSqlQuery &q)
{
	Flight f (q.value (0).toLongLong ());

	f.pilot      =q.value (1).toLongLong ();
	f.copilot    =q.value (2).toLongLong ();
	f.plane      =q.value (3).toLongLong ();
	f.flightType =typeFromDb (
	              q.value (4).toString   ());
	f.mode       =modeFromDb (
	              q.value (5).toString   ());
	f.setStatus  (q.value (6).toInt ());

	f.launchMethod        =q.value ( 7).toLongLong ();
	f.departureAirfield   =q.value ( 8).toString   ();
	f.destinationAirfield =q.value ( 9).toString   ();
	f.numLandings         =q.value (10).toInt      ();
	f.launchTime          =Time::create (
	                       q.value (11).toDateTime (), tz_utc);
	f.landingTime         =Time::create (
	                       q.value (12).toDateTime (), tz_utc);

	f.pvn =q.value (13).toString ();
	f.pnn =q.value (14).toString ();
	f.bvn =q.value (15).toString ();
	f.bnn =q.value (16).toString ();

	f.landingTimeTowflight        =Time::create (
	                               q.value (17).toDateTime (), tz_utc);
	f.modeTowflight               =modeFromDb (
	                               q.value (18).toString   ());
	f.destinationAirfieldTowplane =q.value (19).toString   ();
	f.towplane                    =q.value (20).toLongLong ();

	f.accountingNote =q.value (21).toString ();
	f.comments       =q.value (22).toString ();

	if (opts.record_towpilot)
	{
		f.towpilot =q.value (23).toLongLong ();
		f.tpvn     =q.value (24).toString ();
		f.tpnn     =q.value (25).toString ();
	}

	return f;
}

QString Flight::insertValueList ()
{
	QString columnList=
		"pilot_id,copilot_id,plane_id,type,mode,status" // 6
		",launch_method_id,departure_location,landing_location,num_landings,departure_time,landing_time" // 6 Σ12
		",pilot_first_name,pilot_last_name,copilot_first_name,copilot_last_name" // 4 Σ16
		",towflight_landing_time,towflight_mode,towflight_landing_location,towplane_id" // 4 Σ20
		",accounting_notes,comments" // 2 Σ22
		",towpilot_id,towpilot_first_name,towpilot_last_name" // 3 Σ25
		;

	QString placeholderList="?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?";

	return QString ("(%1) values (%2)").arg (columnList).arg (placeholderList);
}


QString Flight::updateValueList ()
{
	return
		"pilot_id=?,copilot_id=?,plane_id=?,type=?,mode=?,status=?" // 6
		",launch_method_id=?,departure_location=?,landing_location=?,num_landings=?,departure_time=?,landing_time=?" // 6 Σ12
		",pilot_first_name=?,pilot_last_name=?,copilot_first_name=?,copilot_last_name=?" // 4 Σ16
		",towflight_landing_time=?,towflight_mode=?,towflight_landing_location=?,towplane_id=?" // 4 Σ20
		",accounting_notes=?,comments=?" // 2 Σ22
		",towpilot_id=?,towpilot_first_name=?,towpilot_last_name=?"; // 3 Σ25
		;
}

void Flight::bindValues (QSqlQuery &q) const
{
	q.addBindValue (pilot);
	q.addBindValue (copilot);
	q.addBindValue (plane);
	q.addBindValue (typeToDb (flightType));
	q.addBindValue (modeToDb (mode));
	q.addBindValue (getStatus ());

	q.addBindValue (launchMethod);
	q.addBindValue (departureAirfield);
	q.addBindValue (destinationAirfield);
	q.addBindValue (numLandings);
	q.addBindValue (launchTime.toUtcQDateTime ());
	q.addBindValue (landingTime.toUtcQDateTime ());

	q.addBindValue (pvn);
	q.addBindValue (pnn);
	q.addBindValue (bvn);
	q.addBindValue (bnn);

	q.addBindValue (landingTimeTowflight.toUtcQDateTime ());
	q.addBindValue (modeToDb (modeTowflight));
	q.addBindValue (destinationAirfieldTowplane);
	q.addBindValue (towplane);

	q.addBindValue (accountingNote);
	q.addBindValue (comments);

	if (opts.record_towpilot)
	{
		q.addBindValue (towpilot);
		q.addBindValue (tpvn);
		q.addBindValue (tpnn);
	}
}

QList<Flight> Flight::createListFromQuery (QSqlQuery &q)
{
	QList<Flight> list;

	while (q.next ())
		list.append (createFromQuery (q));

	return list;
}


// *** Enum mappers
QString Flight::modeToDb (FlightMode mode)
{
	switch (mode)
	{
		case fmLocal   : return "l";
		case fmComing  : return "k";
		case fmLeaving : return "g";
		case fmNone    : return "?";
		// no default
	}

	assert (false);
	return "?";
}

FlightMode Flight::modeFromDb (QString mode)
{
	if      (mode=="l") return fmLocal;
	else if (mode=="k") return fmComing;
	else if (mode=="g") return fmLeaving;
	else                return fmNone;
}

QString Flight::typeToDb (FlightType type)
{
	switch (type)
	{
		case ftNone          : return "?";
		case ftNormal        : return "normal";
		case ftTraining2     : return "training_2";
		case ftTraining1     : return "training_1";
		case ftTow           : return "tow";
		case ftGuestPrivate  : return "guest_private";
		case ftGuestExternal : return "guest_external";
		// no default
	};

	assert (false);
	return "?";
}

FlightType Flight::typeFromDb (QString type)
{
	if      (type=="normal"        ) return ftNormal;
	else if (type=="training_2"    ) return ftTraining2;
	else if (type=="training_1"    ) return ftTraining1;
	else if (type=="tow"           ) return ftTow;
	else if (type=="guest_private" ) return ftGuestPrivate;
	else if (type=="guest_external") return ftGuestExternal;
	else                             return ftNone;
}

// *** Flag accessors
const int Flight::STATUS_STARTED=1;
const int Flight::STATUS_LANDED=2;
const int Flight::STATUS_TOWFLIGHT_LANDED=4;

void Flight::setStatus (int status)
{
	started         = (status & STATUS_STARTED         )!=0;
	landed          = (status & STATUS_LANDED          )!=0;
	towflightLanded = (status & STATUS_TOWFLIGHT_LANDED)!=0;
}

int Flight::getStatus () const
{
	int status=0;
	if (started        ) status |= STATUS_STARTED         ;
	if (landed         ) status |= STATUS_LANDED          ;
	if (towflightLanded) status |= STATUS_TOWFLIGHT_LANDED;
	return status;
}
