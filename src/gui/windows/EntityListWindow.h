#ifndef _EntityListWindow_h
#define _EntityListWindow_h

#include <QPushButton>
#include <QMenuBar>

#include <Qt3Support>
#define QPtrListIterator Q3PtrListIterator

#include "src/data_types.h"
#include "src/db/db_event.h"
#include "src/db/sk_db.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkTable.h"
#include "src/gui/widgets/SkTableItem.h"
#include "src/gui/windows/SkDialog.h"
#include "src/gui/windows/SplashScreen.h"
#include "src/gui/windows/EntityEditWindow.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/Entity.h"

class SplashScreen;

const QColor col_default=QColor (255, 255, 255);

/*
 * A dialog for listing Entity in a table.
 * This class needs redoing, either as template or using polymorphy, to get rid
 * of all the swich ()es.
 */

class EntityListWindow:public SkDialog
{
	Q_OBJECT

	public:
		EntityListWindow (EntityType, QWidget *parent, sk_db *_db, const char *name=NULL, bool modal=FALSE, WFlags f=0, ::SplashScreen *spl=NULL);
		~EntityListWindow ();
		void liste ();

	private:
		sk_db *db;
		EntityType type;
		SkTable *tab;
		QPushButton *but_close;

		SkTableItem *set_table_cell (int row, int col, const QString &text, QColor bg=col_default, db_id id=0);
		void fillInEntity (EntityType t, int row, Entity *);
		void listEntity (EntityType);
		int addEntity (EntityType t, Entity *st);
		void setup_controls ();
		int entityFromId (EntityType, Entity *, db_id id);
		db_id editEntity (EntityType t, Entity *b);
		int deleteEntity (EntityType t, db_id id);

		Entity *newEntity (EntityType t);
		QMenuBar *menu_bar;
		QMenu *menu_datenbank;

		EntityEditWindow *editor_fenster;

		::SplashScreen *ss;

	protected:
		void keyPressEvent (QKeyEvent *e);

	private:
		void table_activated (int row);
		void tabelle_loeschen (int row);
		void reset ();
		bool entityUsed (EntityType, db_id);

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

