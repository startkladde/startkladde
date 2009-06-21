#ifndef sk_flug_h
#define sk_flug_h

#include <sstream>
#include <string>

#include <QDateTime>

#include "src/accessor.h"
#include "src/data_types.h"
#include "src/text.h"
#include "src/gui/dialogs.h"
#include "src/gui/settings.h"
#include "src/model/sk_flugzeug.h"
#include "src/model/sk_person.h"
#include "src/model/startart_t.h"
#include "src/time/sk_time.h"
#include "src/time/sk_time_t.h"

using namespace std;

enum flug_fehler {
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

class sk_flug
{
	public:
		sk_flug ();

		bool fliegt () const;	// Use with care
		bool sfz_fliegt () const;
		bool vorbereitet () const;
		bool starten (bool force=true, bool interactive=false);
		bool landen (bool force=true, bool interactive=false);
		bool schlepp_landen (bool force=true, bool interactive=false);
		bool zwischenlandung (bool force=true, bool interactive=false);
		string typ_string (length_specification lenspec) const;
		sk_time_t flugdauer () const;
		sk_time_t schleppflugdauer () const;
		bool fehlerhaft (sk_flugzeug *fz, sk_flugzeug *sfz, startart_t *sa) const;
		bool schlepp_fehlerhaft (sk_flugzeug *fz, sk_flugzeug *sfz, startart_t *sa) const;
		flug_fehler fehlerchecking (int *, bool check_flug, bool check_schlepp, sk_flugzeug *fz, sk_flugzeug *sfz, startart_t *startart) const;
		string fehler_string (flug_fehler code) const;
		void dump () const;
		bool happened () const;
		bool finished () const;
		void get_towflight (sk_flug *towflight, db_id towplane_id, db_id sa_id) const;

		db_id id;							// ID des Flugs in der Datenbank
		db_id flugzeug;
		db_id pilot;						// ID des Piloten
		db_id begleiter;					// ID des Begleiters
		db_id towpilot;						// ID des Schleppiloten
		sk_time_t startzeit;
		sk_time_t landezeit;
		sk_time_t landezeit_schleppflugzeug;
		db_id startart;				// ID der Startart
		flug_typ flugtyp;					// Typ des Flugs
		string startort;
		string zielort;
		string zielort_sfz;
		int landungen;
		string bemerkungen;
		string abrechnungshinweis;
		bool editierbar;
		flug_modus modus;
		flug_modus modus_sfz;
		string pvn, pnn, bvn, bnn, tpvn, tpnn;			// Dumme Sache f�r den Fall, dass nur ein Nachname/Vorname bekannt ist.
		db_id towplane;
		bool gestartet, gelandet, sfz_gelandet;
		// Whenn adding something here, add it to get_towflight ()

		sk_time_t efftime () const;
		QDate effdatum (time_zone tz=tz_utc) const;


		string pilot_bezeichnung () const;
		string begleiter_bezeichnung () const;
		string towpilot_bezeichnung () const;

		string unvollst_pilot_name () const;
		string unvollst_begleiter_name () const;
		string unvollst_towpilot_name () const;

		bool collective_bb_entry_possible (sk_flug *prev, const sk_flugzeug &plane) const;

		bool flight_lands_here () const;
		bool flight_starts_here () const;

		int sort (sk_flug *other) const;

	private:
		string unvollst_person_name (string nn, string vn) const;
};

template<class T> class data_item
{
	// - data: A pointer to the data structure. Must be valid or NULL. Must be
	//   non-NULL if (item_given && item_ok)
	// - given: whether the item is present. Some items need not be present
	//   regularly (copilot), others might not be present by error.
	// - ok: whether the item could be retrieved from the database. Only
	//   relevant if (item_given).
	// - result: additional information, like a status from retrieving the item
	//   from the database.
	public:
		T *data;
		bool given;
		bool ok;
		int result;

		data_item<T> ():data (NULL), given (false), ok (false), result (0), owner (false) {}
		~data_item<T> () { if (owner) delete data; }
		void set (T *_data, bool _given, bool _ok, int _result) { data=_data; given=_given; ok=_ok; result=_result; }
		RW_ACCESSOR (bool, owner)

	private:
		bool owner;
};

// A container for the additional parts a flight consists of (plane,
// persons...).
class sk_flug_data
{
	public:
		static sk_flug_data owner ()
		{
			sk_flug_data data;
			data.pilot.set_owner (true);
			data.copilot.set_owner (true);
			data.towpilot.set_owner (true);
			data.plane.set_owner (true);
			data.towplane.set_owner (true);
			data.startart.set_owner (true);
			return data;
		}

		data_item<sk_person> pilot;
		data_item<sk_person> copilot;
		data_item<sk_person> towpilot;
		data_item<sk_flugzeug> plane;
		data_item<sk_flugzeug> towplane;
		data_item<startart_t> startart;
};


#endif

