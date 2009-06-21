#ifndef _FlightWindow_h
#define _FlightWindow_h

#include <qframe.h>
#include <cstdio>
#include <qpushbutton.h>
#include <qdatetimeedit.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include <unistd.h>

#include <QtGui/QDateEdit>
#include <QtGui/QListWidget>

#include <Qt3Support>
#define QValueVector Q3ValueVector



#include "src/gui/widgets/SkComboBox.h"
#include "src/gui/widgets/SkTextBox.h"
#include "src/model/sk_flug.h"
#include "src/model/sk_flugzeug.h"
#include "src/logging/messages.h"
#include "src/gui/widgets/sk_time_edit.h"
#include "src/gui/spacing.h"
#include "src/text.h"
#include "src/db/db_proxy.h"
#include "src/gui/widgets/lbl_cbox.h"
#include "src/data_types.h"
#include "src/gui/settings.h"
#include "src/gui/windows/StuffEditWindow.h"
#include "src/gui/windows/SkDialog.h"
#include "src/db/db_event.h"
#include "src/db/sk_db.h"
#include "src/gui/windows/StuffSelectWindow.h"
#include "src/gui/widgets/SkLabel.h"
#include "src/gui/widgets/SkListWidget.h"

using namespace std;

#define NUM_FIELDS 25

enum flight_editor_mode { fe_none, fe_create, fe_edit };

class FlightWindow:public SkDialog
{
	Q_OBJECT

	public:
		FlightWindow (QWidget *parent, sk_db *_db, const char *name=0, bool modal=FALSE, WFlags f=0, QObject *status_dialog=NULL);
		~FlightWindow ();

		// TODO das gehoert privat.
		int go (flight_editor_mode, sk_flug *, QDate *);

		void populate_lists ();
		void read_db ();

		int edit_flight (sk_flug *f);
		int create_flight (QDate *date_to_use=NULL);
		int duplicate_flight (sk_flug *vorlage);

		flight_editor_mode get_mode () { return mode; }
		sk_flug *get_flight_buffer () { return flight; }

	private slots:
		void slot_ok ();
		void slot_later ();
		void slot_cancel ();

		void slot_registration ();
		void slot_registration_in ();
		void slot_flugtyp (int ind);
		void slot_pilot_vn ();
		void slot_pilot_nn ();
		void slot_begleiter_vn ();
		void slot_begleiter_nn ();
		void slot_towpilot_vn ();
		void slot_towpilot_nn ();
		void slot_modus (int ind);
		void slot_startart (int ind);
		void slot_registration_sfz ();
		void slot_registration_sfz_in ();
		void slot_modus_sfz (int ind);
		void slot_gestartet ();
		void slot_startzeit ();
		void slot_gelandet ();
		void slot_landezeit ();
		void slot_sfz_gelandet ();
		void slot_landezeit_sfz ();
		void slot_startort ();
		void slot_zielort ();
		void slot_zielort_sfz ();
		void slot_landungen ();
		void slot_datum ();
		void slot_bemerkung ();
		void slot_abrechnungshinweis ();

		void setup_controls (bool init=false, bool read_only=false, bool repeat=false);
		void accept_date ();

	protected:
		virtual void done (int);

	private:
		void set_time (bool use_time, bool *use_ziel, sk_time_t *zeit_ziel, QDate datum, QTime zeit);
		void set_buttons (bool, string aktion_text="", bool read_only=false);
		bool person_anlegen (db_id *person_id, QString nachname, QString vorname, QString bezeichnung, bool force);
		int widget_index (QWidget *w);
		void enable_widget (int ind, bool en);
		void enable_widget (QWidget *wid, bool en);
		void fehler_eintragen (sk_flug *f, sk_flugzeug *fz, sk_flugzeug *sfz, bool move_focus=false);
		void set_field_error (QWidget *, bool);
		QColor get_default_color (QWidget *w);
		QWidget *get_error_control (flug_fehler error);

		flight_editor_mode mode;
		sk_flug *flight;
		sk_flugzeug *selected_plane;
		sk_flugzeug *selected_towplane;
		int anzahl_pilot, anzahl_begleiter, anzahl_towpilot;	// Kandidaten f�r Pilot/Begleiter/Schleppilot
		db_id original_pilot_id, original_begleiter_id, original_towpilot_id;
		QObject *status_dialog;

		// TODO use list/QPtrList/...
		SkLabel *label[NUM_FIELDS];
		QWidget *edit_widget[NUM_FIELDS];

		SkComboBox *edit_registration;
		QLabel *edit_flugzeug_typ;
		SkComboBox *edit_flug_typ;
		lbl_cbox *edit_pilot_vn;
		lbl_cbox *edit_pilot_nn;
		lbl_cbox *edit_begleiter_vn;
		lbl_cbox *edit_begleiter_nn;
		lbl_cbox *edit_towpilot_vn;
		lbl_cbox *edit_towpilot_nn;
		SkComboBox *edit_startart;
		SkComboBox *edit_registration_sfz;
		QLabel *edit_typ_sfz;
		SkComboBox *edit_modus;
		SkComboBox *edit_modus_sfz;
		sk_time_edit *edit_startzeit;
		sk_time_edit *edit_landezeit;
		sk_time_edit *edit_landezeit_sfz;
		SkComboBox *edit_startort;
		SkComboBox *edit_zielort;
		SkComboBox *edit_zielort_sfz;
		QLineEdit *edit_landungen;
		SkTextBox *edit_bemerkungen;
		SkComboBox *edit_abrechnungshinweis;
		QDateEdit *edit_datum;
		QListWidget *edit_fehler;

		QPushButton *but_ok, *but_cancel, *but_later;

		bool lock_edit_slots;

		bool accept_flight_data (bool spaeter=false);
		void warning_message (const QString &);
		bool check_flight (db_id *, db_id *, db_id *, db_id *, db_id *, bool, QWidget **error_control=NULL);
		bool check_person (db_id *person_id, string vorname, string nachname, string bezeichnung_n, string bezeichnung_a, bool person_required=true, bool check_flying=true, db_id original_id=invalid_id, string *preselection_club=NULL);
		bool check_plane (db_id *plane_id, sk_flugzeug *plane, string registration, string bezeichnung_n, string bezeichnung_a, int seat_guess);
		bool check_plane_flying (db_id plane_id, string registration, string description_n);

		// TODO bessere Datenstrukturen...
		QValueVector<db_id> startarten;	// Startart IDs, same indicies as in edit_startart
		int startart_index (db_id sa);
		int unknown_startart_index;
		db_id original_startart;

		flug_modus *modi;
		flug_modus *sfz_modi;
		int num_modi;
		int num_sfz_modi;
		int modus_index (flug_modus m);
		int sfz_modus_index (flug_modus m);

		flug_typ *flugtypen;
		int num_flugtypen;
		int flugtyp_index (flug_typ t);


		void flug_eintragen (sk_flug *, bool);
		void reset ();
		void namen_aus_datenbank (lbl_cbox *vorname, lbl_cbox *nachname, lbl_cbox *vorname2=NULL, lbl_cbox *nachname2=NULL, lbl_cbox *vorname3=NULL, lbl_cbox *nachname3=NULL);
		void namen_eintragen (lbl_cbox* vorname, lbl_cbox *nachname, namens_teil quelle, int *, db_id *, bool preserve_target_text=false);

		sk_db *db;

		bool disable_error_check;

		QVBoxLayout *backgroundLayout;
		QScrollArea *scrollArea;

	public slots:
		void slot_db_update (db_event *);

	signals:
		void dialog_finished (int);
};

#endif

