#ifndef _StatisticsWindow_h
#define _StatisticsWindow_h

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <Qt3Support> // XXX
#define QPtrList Q3PtrList

#include "src/dataTypes.h"
#include "src/db/Database.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkTable.h"
#include "src/gui/widgets/SkTableItem.h"
#include "src/gui/windows/SkDialog.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/statistics/PlaneLog.h"
#include "src/statistics/PilotLog.h"

/*
 * A statistics window for displaying logbooks and startart statistics.
 */
class StatisticsWindow:public SkDialog
{
	Q_OBJECT

	public:
		StatisticsWindow (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog, Database *_db);
		void bordbuch (QDate datum);
		void flugbuch (QDate datum);
		void sastat (QDate datum);

	private:
		SkTable *tab;
		QPushButton *but_close;
		SkTableItem *set_table_cell (int row, int col, const QString &text, QColor bg=QColor (255, 255, 255));
		void person_flugbuch (QDate date, Person *p, QPtrList<Flight> &flights);
		void fill_sastat (QDate datum);
		Database *db;

		// New functions
		void display_bordbuch_entry (PlaneLogEntry *bbe);
		void display_flugbuch_entry (PilotLogEntry *fbe);
};

#endif

