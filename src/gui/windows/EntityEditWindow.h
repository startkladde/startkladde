#ifndef _EntityEditWindow_h
#define _EntityEditWindow_h

#include <cstdio>

#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QValidator>

#include "src/text.h"
#include "src/db/db_event.h"
#include "src/db/sk_db.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkComboBox.h"
#include "src/gui/widgets/SkTextBox.h"
#include "src/gui/windows/SkDialog.h"
#include "src/logging/messages.h"
#include "src/model/Plane.h"
#include "src/model/Entity.h"

using namespace std;

// TODO move to EntityEditWindow
enum entity_editor_mode { em_none, em_create, em_edit, em_display };

/*
 * An editor dialog for object classes derived from the Entity class.
 */
// TODO use virtual functions to determine controls, use polymorphy instead of
// EntityType. beware slot_registration etc.

class EntityEditWindow:public SkDialog
{
	Q_OBJECT

	public:
		EntityEditWindow (EntityType, QWidget *parent, sk_db *_db, const char *name=NULL, bool modal=FALSE, WFlags f=0, QObject *status_dialog=NULL);
		~EntityEditWindow ();
		int create (Entity *, bool can_change_name=false);
		int edit (Entity *, bool can_change_name=false);
		int disp (Entity *);
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

		void flugzeug_eintragen (Plane *);
		void person_eintragen (Person *);

		void enable_widgets (bool enable);

		EntityType type;
		entity_editor_mode mode;
		Entity *buf;

		int num_fields;
		QLabel **labels;
		QWidget **edit_widgets;

		// Shortcuts/*{{{*/
		SkTextBox *edit_registration;
		SkTextBox *edit_wettkennz;
		SkComboBox *edit_category;
		SkComboBox *edit_typ;
		SkComboBox *edit_club;
		QLineEdit *edit_sitze;
		SkTextBox *edit_vorname;
		SkTextBox *edit_nachname;
		SkTextBox *edit_landesverbandsnummer;
		SkTextBox *edit_bemerkungen;/*}}}*/

		QPushButton *but_save, *but_cancel;

		bool accept_data ();
		bool check_data ();

		sk_db *db;

	public slots:
		void slot_db_update (db_event *);
};

#endif

