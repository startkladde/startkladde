#include "Flight.h"

#include <iostream>

#include "src/config/options.h"

// MURX: dass hier Meldungen angezeigt werden (bool interactive) ist Pfusch.

/* Switch modus template {{{
switch (modus)
{
	case fmod_lokal:
		break;
	case fmod_geht:
		break;
	case fmod_kommt:
		break;
	default:
		log_error ("Unbehandelter Flugmodus");
		break;
}
}}}*/



// TODO Vereinheitlichen der Statusfunktionen untereinander und mit den
// condition-strings
/**
  * Constructs an Flight instance with empty values.
  */
Flight::Flight ()
{
	id=invalid_id;
	flugzeug=invalid_id;
	landungen=invalid_id;
	pilot=begleiter=towpilot=invalid_id;
	startart=invalid_id;
	flugtyp=ft_kein;
	modus=modus_sfz=fmod_kein;
	gestartet=gelandet=sfz_gelandet=false;
	editierbar=false;
	towplane=invalid_id;
}

bool Flight::fliegt () const/*{{{*/
	/*
	 * Determines whether a flight has landed.
	 * Return value:
	 *   - true if the flight is flying
	 *   - false else.
	 */
{
	return happened () && !finished ();
}/*}}}*/

bool Flight::sfz_fliegt () const/*{{{*/
	/*
	 * Determine if the !!Schleppflugzeug is flying.
	 * Return value:
	 *   - true if the !!Schleppflugzeug is flying.
	 *   - false else.
	 */
{
	return happened () && !sfz_gelandet;
}/*}}}*/

bool Flight::vorbereitet () const/*{{{*/
	/*
	 * Check if the flight is prepared.
	 * Return value:
	 *   - true if the flight is prepared.
	 *   - false else.
	 */
{
	return !happened ();
}/*}}}*/

bool Flight::happened () const/*{{{*/
{
	if (starts_here (modus) && gestartet) return true;
	if (lands_here (modus) && gelandet) return true;
	return false;
}
/*}}}*/

bool Flight::finished () const/*{{{*/
{
	return (lands_here (modus)?gelandet:true);
}
/*}}}*/

sk_time_t Flight::flugdauer () const/*{{{*/
	/*
	 * Calculate the flight time of the flight.
	 * Return value:
	 *   - the flight time.
	 */
{
	sk_time_t t;
	if (gestartet && gelandet) t.set_to (startzeit.secs_to (&landezeit));
	//t=t.addSecs (startzeit.secs_to (&landezeit));
	return t;
}/*}}}*/

sk_time_t Flight::schleppflugdauer () const/*{{{*/
	/*
	 * Calculate the flight time of the !!Schleppflug.
	 * Return value:
	 *   - the flight time.
	 */
{
	sk_time_t t;
	if (gestartet && sfz_gelandet) t.set_to (startzeit.secs_to (&landezeit_schleppflugzeug));
		//t=t.addSecs (startzeit.secsTo (landezeit_schleppflugzeug));
	return t;
}/*}}}*/

bool Flight::fehlerhaft (Plane *fz, Plane *sfz, LaunchType *sa) const/*{{{*/
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
	return (fehlerchecking (&i, true, false, fz, sfz, sa)!=ff_ok);
}/*}}}*/

bool Flight::schlepp_fehlerhaft (Plane *fz, Plane *sfz, LaunchType *sa) const/*{{{*/
	/*
	 * Finds out if the !!Schleppflug for this flight (if any) contains an error.
	 * Parameters:
	 *   - fz: the plane data structure for the flight.
	 * Return value:
	 *   - true if any error was found
	 *   - false else.
	 */
{
	int i=0;
	return (fehlerchecking (&i, false, true, fz, sfz, sa)!=ff_ok);
}/*}}}*/

QString Flight::fehler_string (FlightError code) const/*{{{*/
	/*
	 * Makes a QString describing a flight error.
	 * Parameters:
	 *   code: an error code.
	 * Return value:
	 *   the description.
	 */
{
	switch (code)
	{
		case ff_ok: return "Kein Fehler";
		case ff_keine_id: return "Flug hat keine ID";
		case ff_kein_flugzeug: return "Kein Flugzeug angegeben";
		// TODO use person_bezeichnung (flugtyp) (oder wie die hei�t) here
		case ff_pilot_nur_nachname: return "F�r den "+QString (flugtyp==ft_schul_2?"Flugsch�ler":"Piloten")+" ist nur ein Nachname angegeben";
		case ff_pilot_nur_vorname: return  "F�r den "+QString (flugtyp==ft_schul_2?"Flugsch�ler":"Piloten")+" ist nur ein Vorname angegeben";
		case ff_pilot_nicht_identifiziert: return  "Der "+QString (flugtyp==ft_schul_2?"Flugsch�ler":"Pilot")+" ist nicht identifiziert";
		case ff_begleiter_nur_nachname: return "F�r den "+QString (flugtyp==ft_schul_2?"Fluglehrer":"Begleiter")+" ist nur ein Nachname angegeben";
		case ff_begleiter_nur_vorname: return  "F�r den "+QString (flugtyp==ft_schul_2?"Fluglehrer":"Begleiter")+" ist nur ein Vorname angegeben";
		case ff_begleiter_nicht_identifiziert: return  "Der "+QString (flugtyp==ft_schul_2?"Fluglehrer":"Begleiter")+" ist nicht identifiziert";
		case ff_towpilot_nur_nachname: return "F�r den Schleppiloten ist nur ein Nachname angegeben";
		case ff_towpilot_nur_vorname: return  "F�r den Schleppiloten ist nur ein Vorname angegeben";
		case ff_towpilot_nicht_identifiziert: return  "Der Schleppilot ist nicht identifiziert";
		case ff_kein_pilot: return "Kein "+QString (flugtyp==ft_schul_2 || flugtyp==ft_schul_1?"Flugsch�ler":"Pilot")+" angegeben";
		case ff_pilot_gleich_begleiter: return QString (flugtyp==ft_schul_2?"Flugsch�ler und Fluglehrer":"Pilot und Begleiter")+" sind identisch";
		case ff_pilot_gleich_towpilot: return QString (flugtyp==ft_schul_2?"Flugsch�ler":"Pilot")+" und Schlepppilot sind identisch";
		case ff_schulung_ohne_begleiter: return "Doppelsitzige Schulung ohne Fluglehrer";
		case ff_begleiter_nicht_erlaubt: return "Begleiter ist nicht erlaubt";
		case ff_nur_gelandet: return "Flug ist gelandet, aber nicht gestartet";
		case ff_landung_vor_start: return "Landung liegt vor Start";
		case ff_keine_startart: return "Keine Startart angegeben";
		case ff_kein_modus: return "Kein Modus angegeben";
		case ff_kein_sfz_modus: return "Kein Modus f�r den Schleppflug angegeben";
		case ff_kein_flugtyp: return "Kein Flugtyp angegeben";
		case ff_landungen_negativ: return "Negative Anzahl Landungen";
		case ff_landungen_null: return "Flug ist gelandet, aber Anzahl der Landungen ist 0";
		case ff_schlepp_nur_gelandet: return "Schleppflug ist gelandet, aber nicht gestartet";
		case ff_schlepp_landung_vor_start: return "Landung des Schleppflugs liegt vor Start";
		case ff_doppelsitzige_schulung_in_einsitzer: return "Doppelsitzige Schulung in Einsitzer";
		case ff_kein_startort: return "Kein Startort angegeben";
		case ff_kein_zielort: return "Kein Zielort angegeben";
		case ff_kein_zielort_sfz: return "Kein Zielort f�r das Schleppflugzeug angegeben";
		case ff_segelflugzeug_landungen: return "Segelflugzeug macht mehr als eine Landung";
		case ff_segelflugzeug_landungen_ohne_landung: return "Segelflugzeug macht Landungen ohne Landezeit";
		case ff_begleiter_in_einsitzer: return "Begleiter in einsitzigem Flugzeug";
		case ff_gastflug_in_einsitzer: return "Gastflug in einsitzigem Flugzeug";
		case ff_segelflugzeug_selbststart: return "Segelflugzeug im Selbststart";
		case ff_landungen_ohne_start: return "Anzahl Landungen ungleich null ohne Start";
		case ff_startort_gleich_zielort: return "Startort gleich Zielort";
		case ff_kein_schleppflugzeug: return "Schleppflugzeug nicht angegeben";
		case ff_towplane_is_glider: return "Schleppflugzeug ist Segelflugzeug";
		// No default to allow compiler warning
	}

	return "Unbekannter Fehler";
}/*}}}*/

FlightError Flight::fehlerchecking (int *index, bool check_flug, bool check_schlepp, Plane *fz, Plane *sfz, LaunchType *sa) const/*{{{*/
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
	 *   - check_schlepp: whether to check the !!Schleppflug.
	 * Return value:
	 *   - error code, or ff_ok, if no more error is found.
	 */
{
#define CHECK_FEHLER(bereich, bedingung, fehlercode) if ((*index)==(num++)) { (*index)++; if (bereich && bedingung) return fehlercode; }
#define FLUG (check_flug)
#define SCHLEPP (check_schlepp)
	//printf ("Fehlercheckung: %d %s %s\n", *index, check_flug?"flug":"!flug", check_schlepp?"schlepp":"!schlepp");
	int num=0;

	CHECK_FEHLER (FLUG, id_invalid (id), ff_keine_id)
	CHECK_FEHLER (FLUG, id_invalid (flugzeug), ff_kein_flugzeug)
	CHECK_FEHLER (FLUG, sa->get_person_required () && id_invalid (pilot) && pvn.isEmpty () && pnn.isEmpty (), ff_kein_pilot)
	CHECK_FEHLER (FLUG, id_invalid (pilot) && !pvn.isEmpty () && pnn.isEmpty (), ff_pilot_nur_vorname);
	CHECK_FEHLER (FLUG, id_invalid (pilot) && !pnn.isEmpty () && pvn.isEmpty (), ff_pilot_nur_nachname);
	CHECK_FEHLER (FLUG, id_invalid (pilot) && !pnn.isEmpty () && !pvn.isEmpty (), ff_pilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, begleiter_erlaubt (flugtyp) && id_invalid (begleiter) && !bvn.isEmpty () && bnn.isEmpty (), ff_begleiter_nur_vorname);
	CHECK_FEHLER (FLUG, begleiter_erlaubt (flugtyp) && id_invalid (begleiter) && !bnn.isEmpty () && bvn.isEmpty (), ff_begleiter_nur_nachname);
	CHECK_FEHLER (FLUG, begleiter_erlaubt (flugtyp) && id_invalid (begleiter) && !bnn.isEmpty () && !bvn.isEmpty (), ff_begleiter_nicht_identifiziert);
	CHECK_FEHLER (FLUG, begleiter_erlaubt (flugtyp) && pilot!=0 && pilot==begleiter, ff_pilot_gleich_begleiter)
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && id_invalid (towpilot) && !tpvn.isEmpty () && tpnn.isEmpty (), ff_towpilot_nur_vorname);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && id_invalid (towpilot) && !tpnn.isEmpty () && tpvn.isEmpty (), ff_towpilot_nur_nachname);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && id_invalid (towpilot) && !tpnn.isEmpty () && !tpvn.isEmpty (), ff_towpilot_nicht_identifiziert);
	CHECK_FEHLER (FLUG, opts.record_towpilot && sa && sa->is_airtow () && towpilot!=0 && pilot==towpilot, ff_pilot_gleich_towpilot)
	CHECK_FEHLER (FLUG, id_invalid (begleiter) && (flugtyp==ft_schul_2) && bnn.isEmpty () && bvn.isEmpty (), ff_schulung_ohne_begleiter)
	// TODO einsitzige Schulung mit Begleiter
	CHECK_FEHLER (FLUG, begleiter!=0 && !begleiter_erlaubt (flugtyp), ff_begleiter_nicht_erlaubt)
	CHECK_FEHLER (FLUG, starts_here (modus) && lands_here (modus) && gelandet && !gestartet, ff_nur_gelandet)
	CHECK_FEHLER (FLUG, starts_here (modus) && lands_here (modus) && gestartet && gelandet && startzeit>landezeit, ff_landung_vor_start)
	CHECK_FEHLER (FLUG, id_invalid (startart) && gestartet && starts_here (modus), ff_keine_startart)
	CHECK_FEHLER (FLUG, modus==fmod_kein, ff_kein_modus)
	CHECK_FEHLER (FLUG, flugtyp==ft_kein, ff_kein_flugtyp)
	CHECK_FEHLER (FLUG, landungen<0, ff_landungen_negativ)
	CHECK_FEHLER (FLUG, lands_here (modus) && landungen==0 && gelandet, ff_landungen_null)
	CHECK_FEHLER (FLUG, fz && fz->sitze<=1 && flugtyp==ft_schul_2, ff_doppelsitzige_schulung_in_einsitzer)
	CHECK_FEHLER (FLUG, (gestartet || !starts_here (modus)) && eintrag_ist_leer (startort), ff_kein_startort)
	CHECK_FEHLER (FLUG, (gelandet || !lands_here (modus)) && eintrag_ist_leer (zielort), ff_kein_zielort)
	CHECK_FEHLER (FLUG, (sfz_gelandet || !lands_here (modus_sfz)) && eintrag_ist_leer (zielort_sfz), ff_kein_zielort_sfz)
	CHECK_FEHLER (FLUG, fz && fz->category==lfz_segelflugzeug && landungen>1 && !sa->is_airtow (), ff_segelflugzeug_landungen)
	CHECK_FEHLER (FLUG, fz && fz->category==lfz_segelflugzeug && !gelandet && landungen>0 && !sa->is_airtow (), ff_segelflugzeug_landungen_ohne_landung)
	CHECK_FEHLER (FLUG, fz && fz->sitze<=1 && begleiter_erlaubt (flugtyp) && begleiter!=0, ff_begleiter_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->sitze<=1 && flugtyp==ft_gast_privat, ff_gastflug_in_einsitzer)
	CHECK_FEHLER (FLUG, fz && fz->sitze<=1 && flugtyp==ft_gast_extern, ff_gastflug_in_einsitzer)
	//CHECK_FEHLER (FLUG, fz && fz->category==lfz_segelflugzeug && startart==sa_ss, ff_segelflugzeug_selbststart)
	CHECK_FEHLER (FLUG, starts_here (modus) && landungen>0 && !gestartet, ff_landungen_ohne_start)
	CHECK_FEHLER (FLUG, starts_here (modus)!=lands_here (modus) && startort==zielort, ff_startort_gleich_zielort)
	CHECK_FEHLER (FLUG, sa && sa->is_airtow () && !sa->towplane_known () && id_invalid (towplane), ff_kein_schleppflugzeug)
	CHECK_FEHLER (FLUG, sa && sfz && sa->is_airtow () && !sa->towplane_known () && sfz->category==lfz_segelflugzeug, ff_towplane_is_glider);

	return ff_ok;
#undef CHECK_FEHLER
#undef FLUG
#undef SCHLEPP
}/*}}}*/



bool Flight::starten (bool force, bool interactive)/*{{{*/
	/*
	 * Start the flight now.
	 * Parameters:
	 *   - force: whether to start, regardless of whether the flight can be
	 *     started or not.
	 *   - interactive: whether to show warnings if the flight cannot be
	 *     started.
	 * Return value:
	 *   - true if the flight could be started.
	 *   - false else.
	 */
{
	if ((!gestartet && starts_here (modus))|| force)
	{
		startzeit.set_current (true);
		gestartet=true;
		return true;
	}
	else
	{
		if (interactive)
		{
			if (!starts_here (modus)) show_warning ("Der Flug startet nicht hier.", NULL);
			else if (gestartet) show_warning ("Der Flug ist schon gestartet.", NULL);
		}
		return false;
	}
}/*}}}*/

bool Flight::landen (bool force, bool interactive)/*{{{*/
	/*
	 * Land the flight now.
	 * Parameters:
	 *   - force: whether to land, regardless of whether the flight can be
	 *     landed or not.
	 *   - interactive: whether to show warnings if the flight cannot be
	 *     landed.
	 * Return value:
	 *   - true if the flight could be landed.
	 *   - false else.
	 */
{
	if (lands_here (modus) && (((gestartet || !starts_here (modus)) && !gelandet) || force))
	{
		landezeit.set_current (true);
		landungen++;
		gelandet=true;
		if (eintrag_ist_leer (zielort))
		{
			if (starts_here (modus))
				zielort=startort;
			else
				zielort=opts.ort;
		}
		return true;
	}
	else
	{
		if (interactive)
		{
			if (!lands_here (modus)) show_warning ("Der Flug landet nicht hier", NULL);
			else if (!gestartet) show_warning ("Der Flug ist noch nicht gestartet.", NULL);
			else if (gelandet) show_warning ("Der Flug ist schon gelandet.", NULL);
		}
		return false;
	}
}/*}}}*/

bool Flight::schlepp_landen (bool force, bool interactive)/*{{{*/
	/*
	 * Land the !!Schleppflug now.
	 * Parameters:
	 *   - force: whether to land, regardless of whether the !!Schleppflug can
	 *     be landed or not.
	 *   - interactive: whether to show warnings if the !!Schleppflug cannot be
	 *     landed.
	 * Return value:
	 *   - true if the !!Schleppflug could be landed.
	 *   - false else.
	 */
{
	// TODO kein Schleppflug
	if (sfz_fliegt () || force)
	{
		landezeit_schleppflugzeug.set_current (true);
		sfz_gelandet=true;
		if (lands_here (modus_sfz) && eintrag_ist_leer (zielort_sfz)) zielort_sfz=opts.ort;
		return true;
	}
	else
	{
		if (interactive)
		{
			if (!gestartet) show_warning ("Der Flug ist noch nicht gestartet.", NULL);
			if (gelandet) show_warning ("Der Schleppflug ist schon gelandet.", NULL);
		}
		return false;
	}
}/*}}}*/

bool Flight::zwischenlandung (bool force, bool interactive)/*{{{*/
	/*
	 * Make a !!Zwischenlandung now.
	 * Parameters:
	 *   - force: whether to make a !!Zwischenlandung, regardless of whether
	 *     this is possible or not.
	 *   - interactive: whether to show warnings if the !!Zwischenlandung could
	 *     not be performed.
	 * Return value:
	 *   - true if the !!Zwischenlandung could be performed.
	 *   - false else.
	 */
{
	if (((gestartet || !starts_here (modus)) && !gelandet) || force)
	{
		landungen++;
		return true;
	}
	else
	{
		if (interactive)
		{
			if (!(gestartet || !starts_here (modus))) show_warning ("Der Flug ist noch nicht gestartet.", NULL);
			if (gelandet) show_warning ("Der Flug ist schon gelandet.", NULL);
		}
		return false;
	}
}/*}}}*/



QString Flight::typ_string (length_specification lenspec) const/*{{{*/
	/*
	 * Generates a QString describing the type of the flight.
	 * Parameters:
	 *   - lenspec: the length to generate.
	 * Return value:
	 *   - the description.
	 */
{
	return flugtyp_string (flugtyp, lenspec);
}/*}}}*/



sk_time_t Flight::efftime () const/*{{{*/
{
	// TODO this assumes that every flight at least starts or lands here.
	if (starts_here (modus) && gestartet) return startzeit;
	if (lands_here (modus) && gelandet) return landezeit;
	return sk_time_t ();
}
/*}}}*/



QString Flight::pilot_bezeichnung () const/*{{{*/
	/*
	 * Return a description for the pilot.
	 * Return value:
	 *   - the description.
	 */
{
	return t_pilot_bezeichnung (flugtyp);
}/*}}}*/

QString Flight::begleiter_bezeichnung () const/*{{{*/
	/*
	 * Return a description for the copilot.
	 * Return value:
	 *   - the description.
	 */
{
	return t_begleiter_bezeichnung (flugtyp);
}/*}}}*/


QString Flight::unvollst_pilot_name () const/*{{{*/
	/*
	 * Makes the incomplete name of the pilot.
	 * Return value:
	 *   - the name.
	 */
{
	return unvollst_person_name (pnn, pvn);
}
/*}}}*/

QString Flight::unvollst_begleiter_name () const/*{{{*/
	/*
	 * Makes the incomplete name of the copilot.
	 * Return value:
	 *   - the name.
	 */
{
	return unvollst_person_name (bnn, bvn);
}
/*}}}*/

QString Flight::unvollst_towpilot_name () const/*{{{*/
	/*
	 * Makes the incomplete name of the towpilot.
	 * Return value:
	 *   - the name.
	 */
{
	return unvollst_person_name (tpnn, tpvn);
}
/*}}}*/

QString Flight::unvollst_person_name (QString nn, QString vn) const/*{{{*/
	/*
	 * Makes the incomplete name of a person.
	 * Parameters:
	 *   - nn: the last name.
	 *   - vn: the first name.
	 * Return value:
	 *   - the formatted name.
	 */
{
	// MURX: gcc warns about Trigraph ??).
	if (eintrag_ist_leer (nn) && eintrag_ist_leer (vn)) return "-";
	if (eintrag_ist_leer (nn) && !eintrag_ist_leer (vn)) return "(???, "+vn+")";
	if (!eintrag_ist_leer (nn) && eintrag_ist_leer (vn)) return "("+nn+", "+"???"+QString (")");
	// TODO code duplication: displayed name generation from parts
	if (!eintrag_ist_leer (nn) && !eintrag_ist_leer (vn)) return "("+nn+", "+vn+")";
	log_error ("Unreachable code reached in sk_flug::unvollst_person_name ()");
	return "("+QString ("???")+")";
}
/*}}}*/



bool Flight::flight_lands_here () const/*{{{*/
{
	return lands_here (modus);
}
/*}}}*/

bool Flight::flight_starts_here () const/*{{{*/
{
	return starts_here (modus);
}
/*}}}*/



void Flight::get_towflight (Flight *towflight, db_id towplane_id, db_id sa_id) const/*{{{*/
{
	towflight->id=id;										// The tow flight gets the same ID because there
	                                                        // would be no way to get the ID for a given tow
															// flight. The tow flight can be distinguished from
															// the real flight by the flugtyp.
	if (id_invalid (towplane_id))
		towflight->flugzeug=towplane;						// The ID of the tow plane is known to us
	else
		towflight->flugzeug=towplane_id;					// The ID of the tow plane is given as parameter
	towflight->pilot=towpilot;								// The pilot of the tow flight is the towpilot of the towed flight.
	towflight->begleiter=invalid_id;						// There is no tow copilot.
	towflight->startzeit=startzeit;							// The tow flight started the same time as the towed flight.
	towflight->landezeit=landezeit_schleppflugzeug;			// The tow flight landing time is our landezeit_schleppflugzeug.
	towflight->landezeit_schleppflugzeug=sk_time_t (); 		// The tow flight has no tow flight.
	towflight->startart=sa_id;								// The startart of the tow flight is given as a parameter.
	towflight->flugtyp=ft_schlepp;
	towflight->startort=startort;							// The tow flight started the same place as the towed flight.
	towflight->zielort=zielort_sfz;							// The tow flight landing place is our zielort_sfz.
	towflight->zielort_sfz="";
	if (lands_here (modus_sfz))
		towflight->landungen=1;
	else
		towflight->landungen=0;
	towflight->bemerkungen="Schleppflug f�r "+QString::number (id);
	towflight->abrechnungshinweis="";
	towflight->editierbar=false;							// The tow flight is not editable on its own.
	towflight->modus=modus_sfz;
	towflight->modus_sfz=fmod_kein;
	towflight->pvn="";
	towflight->pnn="";
	towflight->bvn="";
	towflight->bnn="";
	towflight->tpvn="";
	towflight->tpnn="";
	towflight->towplane=invalid_id;
	towflight->gestartet=gestartet;
	towflight->gelandet=sfz_gelandet;
	towflight->sfz_gelandet=false;
}
/*}}}*/

bool Flight::collective_bb_entry_possible (Flight *prev, const Plane &plane) const/*{{{*/
{
	// Only allow if the previous flight and the current flight start and land
	// at the same place.
	if (prev->modus!=fmod_lokal || modus!=fmod_lokal) return false;
	if (prev->startort!=prev->zielort) return false;
	if (prev->zielort!=startort) return false;
	if (startort!=zielort) return false;

	// For motor planes: only allow if the flights are towflights.
	if (plane.category==lfz_segelflugzeug || plane.category==lfz_motorsegler)
	{
		return true;
	}
	else
	{
		if (prev->flugtyp==ft_schlepp && flugtyp==ft_schlepp) return true;
		return false;
	}
}
/*}}}*/



void Flight::dump () const/*{{{*/
{
#define DUMP2(l, v) << " " #l ": " << v
#define DUMP(v) DUMP2 (v, v)
#define DUMP_BOOL(v) DUMP2 (v, (v?"true":"false"))
	std::cout << "sk_flug dump:"
		DUMP (id)
		DUMP (flugzeug)
		DUMP (pilot)
		DUMP (towpilot)
		DUMP (begleiter)
		DUMP_BOOL (editierbar)
		<< std::endl;
#undef DUMP
#undef BDUMP
#undef DUMP2
}
/*}}}*/

int Flight::sort (Flight *other) const/*{{{*/
	/*
	 * Return value:
	 *   - >0 if this flight is later
	 *   - <0 if this flight is earlier
	 *   - =0 if the flights are equal
	 */
{
	// Both prepared ==> equal
	if (vorbereitet () && other->vorbereitet  ()) return 0;

	// Prepared flights to the beginning
	if (vorbereitet ()) return 1;
	if (other->vorbereitet ()) return -1;

	// Sort by effective time
	sk_time_t t1=efftime ();
	sk_time_t t2=other->efftime ();
	if (t1>t2) return 1;
	if (t1<t2) return -1;
	return 0;
}
/*}}}*/



QDate Flight::effdatum (time_zone tz) const/*{{{*/
        /*
         * Calculates the effective date, that is the date to use when sorting the
         * table.
         * Parameters:
         *   - tz: the time zone to use.
         * Return value:
         *   - the effective date.
         */
{
        return efftime ().get_qdate (tz);
}/*}}}*/

