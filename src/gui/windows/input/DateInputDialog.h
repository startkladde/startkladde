#ifndef DATEINPUTDIALOG_H
#define DATEINPUTDIALOG_H

#include <QDialog>

#include "ui_DateInputDialog.h"

class DateInputDialog : public QDialog
{
	public:
		DateInputDialog (QWidget *parent = 0, Qt::WindowFlags f=0);
		~DateInputDialog();

		static bool editDate  (QDate *date ,              const QString &title, const QString &text, QWidget *parent);
		static bool editRange (QDate *first, QDate *last, const QString &title, const QString &text, QWidget *parent);

	protected:
		void setup (bool modal, const QString &title, const QString &text, bool rangeEnabled);

	private:
		Ui::DateInputDialogClass ui;
};

#endif // DATEINPUTDIALOG_H
