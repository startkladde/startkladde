#include "DateInputDialog.h"

#include <QPushButton>

DateInputDialog::DateInputDialog (QWidget *parent, Qt::WindowFlags f):
	QDialog(parent, f)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");
}

DateInputDialog::~DateInputDialog()
{

}

void DateInputDialog::setup (bool modal, const QString &title, const QString &text, bool rangeEnabled)
{
	setModal (modal);

	// Setup window title and date text label
	setWindowTitle (title);
	ui.dateLabel->setText (text);

	// Enable or disable the "range" option
	ui.dateRangeSelect ->setVisible (rangeEnabled);
	ui.firstDateInput  ->setVisible (rangeEnabled);
	ui.dateRangeToLabel->setVisible (rangeEnabled);
	ui.lastDateInput   ->setVisible (rangeEnabled);

	// Enable the correct date input(s)
	ui.singleDateInput->setEnabled (ui.otherDateSelect->isChecked ());

	ui.firstDateInput ->setEnabled (ui.dateRangeSelect->isChecked ());
	ui.lastDateInput  ->setEnabled (ui.dateRangeSelect->isChecked ());
}

bool DateInputDialog::editDate  (QDate *date, const QString &title, const QString &text, QWidget *parent)
{
	DateInputDialog dialog (parent);
	dialog.setup (true, title, text, false);

	QDate today=QDate::currentDate ();
	QDate yesterday=today.addDays (-1);

	dialog.ui.singleDateInput->setDate (*date);

	if (*date == today)
	{
		// Today - use "today"
		dialog.ui.todaySelect->setChecked (true);
		dialog.ui.todaySelect->setFocus ();
	}
	else if (*date == yesterday)
	{
		// Yesterday - use "yesterday"
		dialog.ui.yesterdaySelect->setChecked (true);
		dialog.ui.yesterdaySelect->setFocus ();
	}
	else
	{
		// Neither today nor yesterday - use "other"
		dialog.ui.otherDateSelect->setChecked (true);
		dialog.ui.singleDateInput->setFocus ();
	}

	if (QDialog::Accepted==dialog.exec ())
	{
		if (dialog.ui.todaySelect->isChecked ())
			*date=today;
		else if (dialog.ui.yesterdaySelect->isChecked ())
			*date=yesterday;
		else if (dialog.ui.otherDateSelect->isChecked ())
			*date=dialog.ui.singleDateInput->date ();
		else
			return false;

		return true;
	}
	else
	{
		return false;
	}

}

bool DateInputDialog::editRange (QDate *first, QDate *last, const QString &title, const QString &text, QWidget *parent)
{
	DateInputDialog dialog (parent);
	dialog.setup (true, title, text, true);

}
