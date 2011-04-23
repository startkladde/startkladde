#ifndef DATETIMEINPUTDIALOG_H
#define DATETIMEINPUTDIALOG_H

#include <QDialog>

#include "ui_DateTimeInputDialog.h"

class DateTimeInputDialog : public QDialog
{
	public:
		DateTimeInputDialog (QWidget *parent = 0, Qt::WindowFlags f=0);
		~DateTimeInputDialog();

		static bool editDateTime (QWidget *parent, QDate *date, QTime *time, QString title);

	private:
		Ui::DateTimeInputDialogClass ui;
};

#endif
