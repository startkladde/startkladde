#include "DateInputDialog.h"

DateInputDialog::DateInputDialog (QWidget *parent, Qt::WindowFlags f):
	QDialog(parent, f)
{
	ui.setupUi(this);
}

DateInputDialog::~DateInputDialog()
{

}

bool DateInputDialog::editDate (QWidget *parent, QDate *date, QTime *time, QString title, QString text, bool todayEnabled, bool yesterdayEnabled, bool specificSelected)
{
	DateInputDialog dialog (parent);
	dialog.setModal (true);

	dialog.setWindowTitle (title);
	dialog.ui.dateLabel->setText (text);

	dialog.ui.todaySelect->setVisible (todayEnabled);
	dialog.ui.yesterdaySelect->setVisible (yesterdayEnabled);

	// The "other" field is available if "today" or "yesterday" are available -
	// otherwise, there is only one choice.
	dialog.ui.otherDateSelect->setVisible (todayEnabled || yesterdayEnabled);

	QDate today=QDate::currentDate ();

	if (specificSelected && date)
	{
		// Specific selection active if the date is today or yesterday and the
		// corresponding option is available
		if (todayEnabled && *date == today)
		{
			// Today - use "today"
			dialog.ui.todaySelect->setChecked (true);
			dialog.ui.todaySelect->setFocus ();
		}
		else if (yesterdayEnabled && *date == today.addDays (-1))
		{
			// Yesterday - use "yesterday"
			dialog.ui.yesterdaySelect->setChecked (true);
			dialog.ui.yesterdaySelect->setFocus ();
		}
		else
		{
			// Neither today nor yesterday - use "other"
			dialog.ui.otherDateSelect->setChecked (true);
			dialog.ui.dateInput->setFocus ();
		}
	}
	else
	{
		// Specific selection not active - use "other"
		dialog.ui.otherDateSelect->setChecked (true);
		dialog.ui.dateInput->setFocus ();
	}

	dialog.ui.dateInput->setEnabled (dialog.ui.otherDateSelect->isChecked ());

	if (date)
		dialog.ui.dateInput->setSelectedDate (*date);

	if (time)
		dialog.ui.timeInput->setTime (*time);
	else
	{
		dialog.ui.timeLabel->setVisible (false);
		dialog.ui.timeWidget->setVisible (false);
	}

	// Note that we call exec even if date is NULL
	if (QDialog::Accepted==dialog.exec ())
	{
		if (date)
		{
			if (dialog.ui.todaySelect->isChecked ())
				*date=QDate::currentDate ();
			else if (dialog.ui.yesterdaySelect->isChecked ())
				*date=QDate::currentDate ().addDays (-1);
			else
				*date=dialog.ui.dateInput->selectedDate ();
		}

		if (time)
		{
			*time=dialog.ui.timeInput->time ();
		}

		return true;
	}
	else
	{
		return false;
	}
}
