#ifndef _StuffListWindow_h
#define _StuffListWindow_h

#include <QPushButton>
#include <QMenuBar>

#include <Qt3Support> // XXX
#define QPtrListIterator Q3PtrListIterator

#include "src/data_types.h"
#include "src/db/db_event.h"
#include "src/db/sk_db.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkTable.h"
#include "src/gui/widgets/SkTableItem.h"
#include "src/gui/windows/SkDialog.h"
#include "src/gui/windows/SplashScreen.h"
#include "src/gui/windows/StuffEditWindow.h"
#include "src/model/sk_flugzeug.h"
#include "src/model/sk_person.h"
#include "src/model/stuff.h"

class SplashScreen;

using namespace std;

const QColor col_default=QColor (255, 255, 255);

/*
 * A dialog for listing stuff in a table.
 * This class needs redoing, either as template or using polymorphy, to get rid
 * of all the swich ()es.
 */

class StuffListWindow:public SkDialog
{
	Q_OBJECT

	public:
		StuffListWindow (stuff_type, QWidget *parent, sk_db *_db, const char *name=NULL, bool modal=FALSE, WFlags f=0, ::SplashScreen *spl=NULL);
		~StuffListWindow ();
		void liste ();

	private:
		sk_db *db;
		stuff_type type;
		SkTable *tab;
		QPushButton *but_close;

		SkTableItem *set_table_cell (int row, int col, const string &text, QColor bg=col_default, db_id id=0);
		void stuff_eintragen (stuff_type t, int row, stuff *);
		void list_stuff (stuff_type);
		int add_stuff (stuff_type t, stuff *st);
		void setup_controls ();
		int stuff_aus_id (stuff_type, stuff *, db_id id);
		db_id stuff_editieren (stuff_type t, stuff *b);
		int stuff_loeschen (stuff_type t, db_id id);

		stuff *stuff_new (stuff_type t);
		QMenuBar *menu_bar;
		QMenu *menu_datenbank;

		StuffEditWindow *editor_fenster;

		::SplashScreen *ss;

	protected:
		void keyPressEvent (QKeyEvent *e);

	private:
		void table_activated (int row);
		void tabelle_loeschen (int row);
		void reset ();
		bool stuff_verwendet (stuff_type, db_id);

	private slots:
		void slot_table_double_click (int, int);
		void slot_table_key (int);
		void slot_neu ();
		void slot_loeschen ();
		void slot_refresh ();
		void slot_ok ();
		void slot_abbrechen ();
		void slot_editieren ();

	public slots:
		void slot_db_update (db_event *);
};

#endif

