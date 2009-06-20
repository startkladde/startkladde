#ifndef sk_win_plane_h
#define sk_win_plane_h

#include <cstdio>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qvalidator.h>

#include "src/gui/widgets/sk_combo_box.h"
#include "src/gui/widgets/sk_text_box.h"
#include "src/model/sk_flugzeug.h"
#include "src/logging/messages.h"
#include "src/gui/spacing.h"
#include "src/text.h"
#include "src/model/stuff.h"
#include "src/gui/windows/sk_dialog.h"
#include "src/db/db_event.h"
#include "src/db/sk_db.h"

using namespace std;

enum stuff_editor_mode { em_none, em_create, em_edit, em_display };

/*
 * An editor dialog for object classes derived from the stuff class.
 */
// TODO use virtual functions to determine controls, use polymorphy instead of
// stuff_type. beware slot_registration etc.

class sk_win_stuff_editor:public sk_dialog
{
	Q_OBJECT

	public:
		sk_win_stuff_editor (stuff_type, QWidget *parent, sk_db *_db, const char *name=NULL, bool modal=FALSE, WFlags f=0, QObject *status_dialog=NULL);
		~sk_win_stuff_editor ();
		int create (stuff *, bool can_change_name=false);
		int edit (stuff *, bool can_change_name=false);
		int disp (stuff *);
		void read_db ();
		int exec ();

	private slots:
		void slot_save ();
		void slot_registration ();
	
	private:
		void reset ();
		aircraft_category *categories;
		int num_categories;
		int category_index (aircraft_category gat);
		bool name_editable;

		void setup_controls ();
		void populate_lists ();

		void flugzeug_eintragen (sk_flugzeug *);
		void person_eintragen (sk_person *);

		void enable_widgets (bool enable);

		stuff_type type;
		stuff_editor_mode mode;
		stuff *buf;

		int num_fields;
		QLabel **labels;
		QWidget **edit_widgets;

		// Shortcuts/*{{{*/
		sk_text_box *edit_registration;
		sk_text_box *edit_wettkennz;
		sk_combo_box *edit_category;
		sk_combo_box *edit_typ;
		sk_combo_box *edit_club;
		QLineEdit *edit_sitze;
		sk_text_box *edit_vorname;
		sk_text_box *edit_nachname;
		sk_text_box *edit_landesverbandsnummer;
		sk_text_box *edit_bemerkungen;/*}}}*/

		QPushButton *but_save, *but_cancel;

		bool accept_data ();
		bool check_data ();

		sk_db *db;

	public slots:
		void slot_db_update (db_event *);
};

#endif

