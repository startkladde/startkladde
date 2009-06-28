#ifndef _FlightWindow_h
#define _FlightWindow_h

#include <cstdio>
#include <unistd.h>

#include <QFrame>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QValidator>
#include <QDateEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QDesktopWidget>

#include "src/dataTypes.h"
#include "src/text.h"
#include "src/db/DbEvent.h"
#include "src/db/dbProxy.h"
#include "src/db/Database.h"
#include "src/gui/settings.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkComboBox.h"
#include "src/gui/widgets/SkLabel.h"
#include "src/gui/widgets/SkListWidget.h"
#include "src/gui/widgets/SkTextBox.h"
#include "src/gui/widgets/LabelComboBox.h"
#include "src/gui/widgets/SkTimeEdit.h"
#include "src/gui/windows/SkDialog.h"
#include "src/gui/windows/EntityEditWindow.h"
#include "src/gui/windows/EntitySelectWindow.h"
#include "src/logging/messages.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"

#define NUM_FIELDS 25

enum flight_editor_mode { fe_none, fe_create, fe_edit };

class FlightWindow:public SkDialog
{
	Q_OBJECT

	public:
		FlightWindow (QWidget *parent, Database *_db, const char *name=0, bool modal=FALSE, WFlags f=0, QObject *status_dialog=NULL);
		~FlightWindow ();

		// TODO das gehoert privat.
		int go (flight_editor_mode, Flight *, QDate *);

		void populate_lists ();
		void read_db ();

		int edit_flight (Flight *f);
		int create_flight (QDate *date_to_use=NULL);
		int duplicate_flight (Flight *vorlage);

		flight_editor_mode get_mode () { return mode; }
		Flight *get_flight_buffer () { return flight; }

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
		void set_time (bool use_time, bool *use_ziel, Time *zeit_ziel, QDate datum, QTime zeit);
		void set_buttons (bool, QString aktion_text="", bool read_only=false);
		bool person_anlegen (db_id *person_id, QString nachname, QString vorname, QString bezeichnung, bool force);
		int widget_index (QWidget *w);
		void enable_widget (int ind, bool en);
		void enable_widget (QWidget *wid, bool en);
		void fehler_eintragen (Flight *f, Plane *fz, Plane *sfz, bool move_focus=false);
		void set_field_error (QWidget *, bool);
		QColor get_default_color (QWidget *w);
		QWidget *get_error_control (FlightError error);

		flight_editor_mode mode;
		Flight *flight;
		Plane *selected_plane;
		Plane *selected_towplane;
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
		SkTimeEdit *edit_startzeit;
		SkTimeEdit *edit_landezeit;
		SkTimeEdit *edit_landezeit_sfz;
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
		bool check_person (db_id *person_id, QString vorname, QString nachname, QString bezeichnung_n, QString bezeichnung_a, bool person_required=true, bool check_flying=true, db_id original_id=invalid_id, QString *preselection_club=NULL);
		bool check_plane (db_id *plane_id, Plane *plane, QString registration, QString bezeichnung_n, QString bezeichnung_a, int seat_guess);
		bool check_plane_flying (db_id plane_id, QString registration, QString description_n);

		// TODO bessere Datenstrukturen...
		QVector<db_id> startarten;	// Startart IDs, same indicies as in edit_startart
		int startart_index (db_id sa);
		int unknown_startart_index;
		db_id original_startart;

		QList <FlightMode> modi;
		QList <FlightMode> sfz_modi;
		int modus_index (FlightMode m);
		int sfz_modus_index (FlightMode m);

		QList<FlightType> flightTypes;
		int flugtyp_index (FlightType t);


		void flug_eintragen (Flight *, bool);
		void reset ();
		void namen_aus_datenbank (lbl_cbox *vorname, lbl_cbox *nachname, lbl_cbox *vorname2=NULL, lbl_cbox *nachname2=NULL, lbl_cbox *vorname3=NULL, lbl_cbox *nachname3=NULL);
		void namen_eintragen (lbl_cbox* vorname, lbl_cbox *nachname, NamePart quelle, int *, db_id *, bool preserve_target_text=false);

		Database *db;

		bool disable_error_check;

		QVBoxLayout *backgroundLayout;
		QScrollArea *scrollArea;

	public slots:
		void slot_db_update (DbEvent *);

	signals:
		void dialog_finished (int);
};

#endif

