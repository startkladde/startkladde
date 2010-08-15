#ifndef FLIGHT_H_
#define FLIGHT_H_

#include <cassert>

#include <QApplication>
#include <QString>
#include <QDateTime>
#include <QMetaType>

#include "src/db/dbId.h"

class Plane;
class LaunchMethod;

class Query;
class Result;
class Cache;

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

class Database;

// TODO: inherit from Entity
class Flight
{
	friend class Database;

	public:
		// *** Types
		enum Type {
			typeNone,
			typeNormal,
			typeTraining2, typeTraining1,
			typeGuestPrivate, typeGuestExternal,
			typeTow
		};

		enum Mode { modeLocal, modeComing, modeLeaving };


		// *** Construction
		Flight ();
		Flight (dbId id); // TODO protected (friend Database)?


		// *** Data
		dbId planeId, pilotId, copilotId;
		Type type;
		Mode mode;

		bool departed, landed, towflightLanded;
		dbId launchMethodId;
		QString departureLocation;
		QString landingLocation;

		QDateTime departureTime;
		QDateTime landingTime;
		int numLandings;

		dbId towplaneId;
		Mode towflightMode;
		QString towflightLandingLocation;
		QDateTime towflightLandingTime;
		dbId towpilotId;

		// Incomplete names
		QString pilotLastName   , pilotFirstName   ;
		QString copilotLastName , copilotFirstName ;
		QString towpilotLastName, towpilotFirstName;

		QString comments;
		QString accountingNotes;


		// *** Attribute accessors
		virtual dbId getId () const { return id; }
		virtual void setId (dbId id) { this->id=id; } // TODO can we do without this?


		// *** Comparison
		virtual bool operator< (const Flight &o) const;
		static bool lessThan (Flight *a, Flight *b) { return *a < *b; }
		virtual int sort (const Flight *other) const;


		// *** Status
		// TODO fliegt and isFlying are probably not correct
		virtual bool fliegt () const { return happened () && !finished (); }
		virtual bool isFlying () const { return departsHere () && landsHere () && departed && !landed; }
		virtual bool sfz_fliegt () const { return happened () && !towflightLanded; }
//		TODO: !((departs_here and departed) or (lands_here and landed))
		virtual bool isPrepared () const { return !happened (); }
		// TODO this is certainly not correct
		virtual bool isTowplaneFlying () const { return departsHere () && towflightLandsHere () && departed && !towflightLanded; }

		virtual bool happened () const;
		virtual bool finished () const;

		static int countFlying (const QList<Flight> flights);
		static int countHappened (const QList<Flight> flights);


		// *** Crew
		virtual QString pilotDescription () const;
		virtual QString copilotDescription () const;
		virtual QString towpilotDescription () const { return "Schlepppilot"; }

		virtual bool pilotSpecified    () const;
		virtual bool copilotSpecified  () const;
		virtual bool towpilotSpecified () const;

		virtual QString incompletePilotName () const;
		virtual QString incompleteCopilotName () const;
		virtual QString incompleteTowpilotName () const;

		virtual bool copilotRecorded () const { return typeCopilotRecorded (type); }
		virtual bool hasCopilot () const { return typeAlwaysHasCopilot (type) || (typeCopilotRecorded (type) && copilotSpecified ()); }
		virtual int numPassengers () const { return hasCopilot ()?2:1; } // TODO: this is inaccurate for planes with >2 seats


		// *** Launch method
		virtual bool isAirtow (Cache &cache) const;
		virtual dbId effectiveTowplaneId (Cache &cache) const;

		// *** Departure/landing
		virtual bool departsHere        () const { return departsHere (mode         ); }
		virtual bool landsHere          () const { return landsHere   (mode         ); }
		virtual bool towflightLandsHere () const { return landsHere   (towflightMode); }

		virtual bool canDepart        (QString *reason=NULL) const;
		virtual bool canLand          (QString *reason=NULL) const;
		virtual bool canTouchngo      (QString *reason=NULL) const;
		virtual bool canTowflightLand (QString *reason=NULL) const;

		virtual bool departNow        (bool force=false);
		virtual bool landNow          (bool force=false);
		virtual bool landTowflightNow (bool force=false);
		virtual bool performTouchngo  (bool force=false);

		// *** Times
		virtual QDateTime effectiveTime () const;
		// TODO which one of these is right?
		virtual QDate effdatum (Qt::TimeSpec spec=Qt::UTC) const;
		virtual QDate getEffectiveDate (Qt::TimeSpec spec, QDate defaultDate) const;

		virtual bool canHaveDepartureTime        () const { return departsHere (); }
		virtual bool canHaveLandingTime          () const { return landsHere () || isTowflight (); }
		virtual bool canHaveTowflightLandingTime () const { return true; } // Leaving towflights hava an end time

		virtual bool hasDepartureTime        () const { return canHaveDepartureTime        () && departed       ; }
		virtual bool hasLandingTime          () const { return canHaveLandingTime          () && landed         ; }
		virtual bool hasTowflightLandingTime () const { return canHaveTowflightLandingTime () && towflightLanded; }

		virtual QTime flightDuration () const;
		virtual QTime towflightDuration () const;

		// TODO not good - hasDepartureTime and canHaveLandingTime; flying flights already have a duration!
		virtual bool hasDuration () const { return hasDepartureTime () && canHaveLandingTime (); }
		virtual bool hasTowflightDuration () const { return hasDepartureTime () && canHaveTowflightLandingTime (); }


		// *** Error checking
		virtual FlightError errorCheck (int *, bool check_flug, bool check_schlepp, Plane *fz, Plane *sfz, LaunchMethod *launchMethod) const;
		virtual QString errorDescription (FlightError code) const;
		virtual bool isErroneous (Cache &cache) const;
		virtual bool fehlerhaft (Plane *fz, Plane *sfz, LaunchMethod *sa, QString *errorText=NULL) const;
		virtual bool schlepp_fehlerhaft (Plane *fz, Plane *sfz, LaunchMethod *sa, QString *errorText=NULL) const;


		// *** Formatting
		virtual QString toString () const;


		// *** Misc
		virtual bool collectiveLogEntryPossible (const Flight *prev, const Plane *plane) const;
		virtual bool isExternal () const { return !landsHere () || !departsHere (); }
		virtual Flight makeTowflight (dbId theTowplaneId, dbId towLaunchMethod) const;
		static QList<Flight> makeTowflights (const QList<Flight> &flights, Cache &cache);
		QColor getColor (Cache &cache) const;
		virtual bool isTraining () const { return typeIsTraining (type); }

		// TODO: this concept is bad - a flight in the database must never
		// have the flight type "towflight", because that is reserved for
		// "shadow" towflights created from flights from the database; when
		// the user performs "land" on a towflight, it does not land the flight
		// with that ID but its towflight.
		// The towflights should probably be separate flights in the database.
		virtual bool isTowflight () const { return type==typeTow; }


		// *** Type methods
		static QList<Type> listTypes (bool includeInvalid);
		static QString typeText (Type type, bool withShortcut=false);
		static QString shortTypeText (Type type);
		static bool typeCopilotRecorded (Type type);
		static bool typeAlwaysHasCopilot (Type type);
		static QString typePilotDescription (Type type);
		static QString typeCopilotDescription (Type type);
		static bool typeIsGuest (Type type);
		static bool typeIsTraining (Type type);


		// *** Mode methods
		static QList<Mode> listModes ();
		static QList<Mode> listTowModes ();
		static QString modeText (Mode mode);
		static bool landsHere (Mode mode);
		static bool departsHere (Mode mode);


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return "Flug"; }
		static QString objectTypeDescriptionDefinite () { return "der Flug"; }
		static QString objectTypeDescriptionPlural () { return qApp->translate ("Flight", "Flüge"); }


		// *** SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Flight createFromResult (const Result &result);
		static QString insertColumnList ();
		static QString insertPlaceholderList ();
		virtual void bindValues (Query &q) const;
		static QList<Flight> createListFromResult (Result &result);

		// Enum mappers
		static QString    modeToDb   (Mode       mode);
		static Mode       modeFromDb (QString    mode);
		static QString    typeToDb   (Type       type);
		static Type       typeFromDb (QString    type);

		// Queries
		static Query referencesPersonCondition (dbId id);
		static Query referencesPlaneCondition (dbId id);
		static Query referencesLaunchMethodCondition (dbId id);

	private:
		dbId id;

		virtual void initialize (dbId id);
		virtual QString incompletePersonName (QString nn, QString vn) const;
};

Q_DECLARE_METATYPE (Flight);

#endif

