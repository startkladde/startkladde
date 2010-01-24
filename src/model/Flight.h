#ifndef _Flight_h
#define _Flight_h

#include <sstream>
#include <QString>

#include <qdatetime.h>

#include "src/accessor.h"
#include "src/dataTypes.h"
#include "src/text.h"
#include "src/gui/dialogs.h"
#include "src/gui/settings.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchType.h"
#include "src/time/Time.h"
#include "src/time/timeFunctions.h"

class DataStorage;

enum FlightError {
	ff_ok,
	ff_keine_id, ff_kein_flugzeug, ff_kein_pilot, ff_pilot_gleich_begleiter,
	ff_pilot_gleich_towpilot,
	ff_pilot_nur_vorname, ff_pilot_nur_nachname, ff_pilot_nicht_identifiziert,
	ff_begleiter_nur_vorname, ff_begleiter_nur_nachname,
	ff_begleiter_nicht_identifiziert,
	ff_towpilot_nur_vorname, ff_towpilot_nur_nachname,
	ff_towpilot_nicht_identifiziert,
	ff_schulung_ohne_begleiter,
	ff_begleiter_nicht_erlaubt, ff_nur_gelandet, ff_landung_vor_start,
	ff_keine_startart, ff_kein_modus, ff_kein_sfz_modus, ff_kein_flugtyp,
	ff_landungen_negativ, ff_doppelsitzige_schulung_in_einsitzer,
	ff_kein_startort, ff_kein_zielort, ff_kein_zielort_sfz,
	ff_segelflugzeug_landungen, ff_begleiter_in_einsitzer,
	ff_gastflug_in_einsitzer, ff_segelflugzeug_selbststart,
	ff_schlepp_nur_gelandet, ff_schlepp_landung_vor_start, ff_landungen_null,
	ff_landungen_ohne_start, ff_segelflugzeug_landungen_ohne_landung,
	ff_startort_gleich_zielort, ff_kein_schleppflugzeug, ff_towplane_is_glider
	};

class Flight
{
	public:
		Flight ();

		bool operator< (const Flight &o) const;
		static bool lessThan (Flight *a, Flight *b) { return *a < *b; }

		bool fliegt () const;	// Use with care
		bool sfz_fliegt () const;
		bool vorbereitet () const;

		bool startNow (bool force=false);
		bool landNow (bool force=false);
		bool landTowflightNow (bool force=false);
		bool performTouchngo (bool force=false);

		QString typeString (lengthSpecification lenspec) const;
		Time flightDuration () const;
		Time towflightDuration () const;
		bool fehlerhaft (Plane *fz, Plane *sfz, LaunchType *sa, QString *errorText=NULL) const;
		bool schlepp_fehlerhaft (Plane *fz, Plane *sfz, LaunchType *sa, QString *errorText=NULL) const;
		FlightError errorCheck (int *, bool check_flug, bool check_schlepp, Plane *fz, Plane *sfz, LaunchType *startart) const;
		QString errorDescription (FlightError code) const;
		void dump () const;
		bool happened () const;
		bool finished () const;
		Flight makeTowflight (db_id towplaneId, db_id towLaunchType) const;
		void get_towflight (Flight *towflight, db_id towplaneId, db_id towLaunchType) const;
		QString toString () const;
		bool isExternal () const { return !lands_here (mode) || !starts_here (mode); }

		db_id id;							// ID des Flugs in der Datenbank
		db_id plane;
		db_id pilot;						// ID des Piloten
		db_id copilot;					// ID des Begleiters
		db_id towpilot;						// ID des Schleppiloten
		Time launchTime;
		Time landingTime;
		Time landingTimeTowflight;
		db_id launchType;				// ID der Startart
		FlightType flightType;					// Typ des Flugs
		QString departureAirfield;
		QString destinationAirfield;
		QString destinationAirfieldTowplane;
		int numLandings;
		QString comments;
		QString accountingNote;
		bool editable;
		FlightMode mode;
		FlightMode modeTowflight;
		QString pvn, pnn, bvn, bnn, tpvn, tpnn;			// Dumme Sache für den Fall, dass nur ein Nachname/Vorname bekannt ist.
		db_id towplane;
		bool started, landed, towflightLanded;
		// Whenn adding something here, addObject it to get_towflight ()


		db_id get_id () const { return id; }

		Time effectiveTime () const;
		// TODO which one of these is right?
		QDate effdatum (time_zone tz=tz_utc) const;
		QDate getEffectiveDate (time_zone tz, QDate defaultDate) const;

		QString pilotDescription () const;
		QString copilotDescription () const;
		QString towpilotDescription () const;

		QString incompletePilotName () const;
		QString incompleteCopilotName () const;
		QString incompleteTowpilotName () const;

		bool collective_bb_entry_possible (const Flight *prev, const Plane *plane) const;

		bool flight_lands_here () const;
		bool flight_starts_here () const;

		int sort (const Flight *other) const;

		// Convenience functions
		bool pilotSpecified    () const { return id_valid (pilot)     || !eintraege_sind_leer (pvn , pnn ); }
		bool copilotSpecified  () const { return id_valid (copilot) || !eintraege_sind_leer (bvn , bnn ); }
		bool towpilotSpecified () const { return id_valid (towpilot)  || !eintraege_sind_leer (tpvn, tpnn); }

		// TODO: this concept is bad - a flight in the database must never
		// have the flight type "towflight", because that is reserved for
		// "shadow" towflights created from flights from the database; when
		// the user performs "land" on a towflight, it does not land the flight
		// with that ID but its towflight.
		bool isTowflight () const { return flightType==ftTow; }

		bool hasCopilot () const { return flightTypeAlwaysHasCopilot (flightType) || (flightTypeCopilotRecorded (flightType) && copilotSpecified ()); }
		int numPassengers () const { return hasCopilot ()?2:1; } // TODO: this is inaccurate for planes with >2 seats

		bool startsHere () const { return starts_here (mode); }
		bool landsHere () const { return lands_here (mode); }
		bool towflightLandsHere () const { return lands_here (modeTowflight); }

		bool canStart (QString *reason=NULL) const;
		bool canLand (QString *reason=NULL) const;
		bool canTouchngo (QString *reason=NULL) const;
		bool canTowflightLand (QString *reason=NULL) const;

		bool canHaveStartTime () const { return startsHere (); }
		bool canHaveLandingTime () const { return landsHere () || isTowflight (); }
		bool canHaveTowflightLandingTime () const { return true; } // Going towflights hava an end time

		bool hasStartTime () const { return canHaveStartTime () && started; }
		bool hasLandingTime () const { return canHaveLandingTime () && landed; }
		bool hasTowflightLandingTime () const { return canHaveTowflightLandingTime () && towflightLanded; }

		// TODO not good
		bool hasDuration () const { return hasStartTime () && canHaveLandingTime (); }
		bool hasTowflightDuration () const { return hasStartTime () && canHaveTowflightLandingTime (); }

		// TODO this is certainly not correct
		bool isFlying () const { return startsHere () && landsHere () && started && !landed; }
		bool isTowplaneFlying () const { return startsHere () && towflightLandsHere () && started && !towflightLanded; }

		static int countFlying (const QList<Flight> flights);
		static int countHappened (const QList<Flight> flights);

		static QString objectTypeDescription () { return "Flug"; }
		static QString objectTypeDescriptionDefinite () { return "der Flug"; }
		static QString objectTypeDescriptionPlural () { return QString::fromUtf8 ("Flüge"); }

		bool isErroneous (DataStorage &dataStorage) const;

	private:
		QString incompletePersonName (QString nn, QString vn) const;
};

#endif

