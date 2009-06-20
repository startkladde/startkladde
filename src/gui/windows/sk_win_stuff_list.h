#ifndef sk_win_stuff_list_h
#define sk_win_stuff_list_h

#include <qpushbutton.h>
#include <qmenubar.h>

// Qt3:
//#include <qtable.h>
//#include <qpopupmenu.h>
// Qt4:
#include <Qt3Support>
#define QPopupMenu Q3PopupMenu
#define QPtrListIterator Q3PtrListIterator

#include "src/gui/spacing.h"
#include "src/model/sk_flugzeug.h"
#include "src/model/sk_person.h"
#include "src/gui/widgets/sk_table_item.h"
#include "src/data_types.h"
#include "src/model/stuff.h"
#include "src/gui/widgets/sk_table.h"
#include "src/gui/windows/sk_win_stuff_editor.h"
#include "src/gui/windows/sk_dialog.h"
#include "src/db/db_event.h"
#include "src/db/sk_db.h"
#include "src/gui/windows/splash.h"

using namespace std;

const QColor col_default=QColor (255, 255, 255);

/*
 * A dialog for listing stuff in a table.
 * This class needs redoing, either as template or using polymorphy, to get rid
 * of all the swich ()es.
 */

class sk_win_stuff_list:public sk_dialog
{
	Q_OBJECT

	public:
		sk_win_stuff_list (stuff_type, QWidget *parent, sk_db *_db, const char *name=NULL, bool modal=FALSE, WFlags f=0, splash *spl=NULL);
		~sk_win_stuff_list ();
		void liste ();
	
	private:
		sk_db *db;
		stuff_type type;
		sk_table *tab;
		QPushButton *but_close;

		sk_table_item *set_table_cell (int row, int col, const string &text, QColor bg=col_default, db_id id=0);
		void stuff_eintragen (stuff_type t, int row, stuff *);
		void list_stuff (stuff_type);
		int add_stuff (stuff_type t, stuff *st);
		void setup_controls ();
		int stuff_aus_id (stuff_type, stuff *, db_id id);
		db_id stuff_editieren (stuff_type t, stuff *b);
		int stuff_loeschen (stuff_type t, db_id id);
		
		stuff *stuff_new (stuff_type t);
		QMenuBar *menu_bar;
		QPopupMenu *menu_datenbank;

		sk_win_stuff_editor *editor_fenster;

		splash *ss;
	
	protected:
		void keyPressEvent (QKeyEvent *e);
	
	private:
		void table_activated (int row);
		void tabelle_loeschen (int row);
		void reset ();
		bool stuff_verwendet (stuff_type, db_id);

	private slots:
		void slot_table_double_click (int, int, int, const QPoint &);
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

