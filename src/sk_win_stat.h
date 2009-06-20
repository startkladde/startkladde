#ifndef sk_win_stat_h
#define sk_win_stat_h

#include <qdialog.h>
#include <qpushbutton.h>

#include "spacing.h"
#include "sk_flug.h"
#include "sk_flugzeug.h"
#include "sk_person.h"
#include "sk_table_item.h"
#include "data_types.h"
#include "sk_table.h"
#include "sk_db.h"
#include "sk_dialog.h"
#include <qapplication.h>
#include "bordbuch.h"
#include "flugbuch.h"

// Qt3:
//#include "qptrlist.h"
// Qt4:
#include <Qt3Support>
#define QPtrList Q3PtrList

using namespace std;

/*
 * A statistics window for displaying logbooks and startart statistics.
 */
class sk_win_stat:public sk_dialog
{
	Q_OBJECT
	
	public:
		sk_win_stat (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog, sk_db *_db);
		void bordbuch (QDate datum);
		void flugbuch (QDate datum);
		void sastat (QDate datum);
	
	private:
		sk_table *tab;
		QPushButton *but_close;
		sk_table_item *set_table_cell (int row, int col, const string &text, QColor bg=QColor (255, 255, 255));
		void person_flugbuch (QDate date, sk_person *p, QPtrList<sk_flug> &flights);
		void fill_sastat (QDate datum);
		sk_db *db;

		// New functions
		void display_bordbuch_entry (bordbuch_entry *bbe);
		void display_flugbuch_entry (flugbuch_entry *fbe);
};

#endif

