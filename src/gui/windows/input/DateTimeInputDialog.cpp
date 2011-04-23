#include "DateTimeInputDialog.h"

#include <QPushButton>

DateTimeInputDialog::DateTimeInputDialog (QWidget *parent, Qt::WindowFlags f):
	QDialog(parent, f)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Cancel)->setText ("Abbre&chen");
}

DateTimeInputDialog::~DateTimeInputDialog()
{

}

bool DateTimeInputDialog::editDateTime (QWidget *parent, QDate *date, QTime *time, QString title)
{
	DateTimeInputDialog dialog (parent);
	dialog.setModal (true);

	dialog.setWindowTitle (title);

	dialog.ui.dateInput->setDate (*date);
	dialog.ui.timeInput->setTime (*time);

	if (QDialog::Accepted==dialog.exec ())
	{
		*date=dialog.ui.dateInput->date ();
		*time=dialog.ui.timeInput->time ();

		return true;
	}
	else
	{
		return false;
	}
}
