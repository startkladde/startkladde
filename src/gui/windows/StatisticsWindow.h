#ifndef _StatisticsWindow_h
#define _StatisticsWindow_h

#include <qdialog.h>
#include <qpushbutton.h>

#include "src/gui/spacing.h"
#include "src/model/sk_flug.h"
#include "src/model/sk_flugzeug.h"
#include "src/model/sk_person.h"
#include "src/gui/widgets/sk_table_item.h"
#include "src/data_types.h"
#include "src/gui/widgets/sk_table.h"
#include "src/db/sk_db.h"
#include "src/gui/windows/SkDialog.h"
#include <qapplication.h>
#include "src/statistics/bordbuch.h"
#include "src/statistics/flugbuch.h"

// Qt3:
//#include "qptrlist.h"
// Qt4:
#include <Qt3Support>
#define QPtrList Q3PtrList

using namespace std;

/*
 * A statistics window for displaying logbooks and startart statistics.
 */
class StatisticsWindow:public SkDialog
{
	Q_OBJECT

	public:
		StatisticsWindow (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog, sk_db *_db);
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

