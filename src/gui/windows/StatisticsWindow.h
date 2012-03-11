#ifndef STATISTICSWINDOW_H
#define STATISTICSWINDOW_H

#include "ui_StatisticsWindow.h"

#include "src/gui/SkDialog.h"

class QAbstractTableModel;

class StatisticsWindow: public SkDialog<Ui::StatisticsWindowClass>
{
	public:
		StatisticsWindow (QAbstractTableModel *model, bool modelOwned, QWidget *parent=0);
		~StatisticsWindow ();

		static void display (QAbstractTableModel *model, bool modelOwned, QString title, QWidget *parent=0);

	private:
		QAbstractTableModel *model;
		bool modelOwned;
};

#endif // STATISTICSWINDOW_H
