#ifndef _StatisticsWindow_h
#define _StatisticsWindow_h

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <Qt3Support> // XXX
#define QPtrList Q3PtrList

#include "src/data_types.h"
#include "src/db/sk_db.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkTable.h"
#include "src/gui/widgets/SkTableItem.h"
#include "src/gui/windows/SkDialog.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/statistics/bordbuch.h"
#include "src/statistics/flugbuch.h"

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
		SkTable *tab;
		QPushButton *but_close;
		SkTableItem *set_table_cell (int row, int col, const QString &text, QColor bg=QColor (255, 255, 255));
		void person_flugbuch (QDate date, Person *p, QPtrList<Flight> &flights);
		void fill_sastat (QDate datum);
		sk_db *db;

		// New functions
		void display_bordbuch_entry (bordbuch_entry *bbe);
		void display_flugbuch_entry (flugbuch_entry *fbe);
};

#endif

