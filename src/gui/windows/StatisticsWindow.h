#ifndef STATISTICSWINDOW_H
#define STATISTICSWINDOW_H

#include <QtGui/QDialog>

#include "ui_StatisticsWindow.h"

class QAbstractTableModel;

class StatisticsWindow: public QDialog
{
	public:
		StatisticsWindow (QAbstractTableModel *model, bool modelOwned, QWidget *parent=0);
		~StatisticsWindow ();

		static void display (QAbstractTableModel *model, bool modelOwned, QString title, QWidget *parent=0);

	private:
		Ui::StatisticsWindowClass ui;

		QAbstractTableModel *model;
		bool modelOwned;
};

#endif // STATISTICSWINDOW_H
