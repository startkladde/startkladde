#include "TaskProgressDialog.h"

#include <iostream>

#include "src/concurrent/task/Task.h"

TaskProgressDialog::TaskProgressDialog (QWidget *parent, Task *task):
    QDialog(parent), task (task)
{
	ui.setupUi(this);
}

TaskProgressDialog::~TaskProgressDialog()
{

}

void TaskProgressDialog::reject ()
{
	ui.statusLabel->setText ("Abbrechen...");
	task->cancel ();
}

void TaskProgressDialog::progress (int progress, int maxProgress)
{
	ui.progressBar->setValue (progress);
	if (maxProgress>=0)
		ui.progressBar->setMaximum (maxProgress);
}

void TaskProgressDialog::status (QString status)
{
	ui.statusLabel->setText (status);
}

void TaskProgressDialog::waitTask (QWidget *parent, Task *task)
{
	TaskProgressDialog *dialog=new TaskProgressDialog (parent, task);

	dialog->ui.statusLabel->setText (task->toString ());

	QObject::connect (task, SIGNAL (ended ())                  , dialog, SLOT (accept ()          ));
	QObject::connect (task, SIGNAL (progressChanged (int, int)), dialog, SLOT (progress (int, int)));
	QObject::connect (task, SIGNAL (statusChanged (QString))   , dialog, SLOT (status (QString)   ));

	// Note that the ended signal is delivered to accept() by the event loop,
	// so it cannot happen between the isEnded check and the call to exec. So
	// this is not a race condition.
	if (!task->isEnded ())
		dialog->exec ();

	delete dialog;

	// Here's how to do it with a QProgressDialog:
	//	QProgressDialog progress ("Warten...", "Abbrechen", 0, task->getMaxProgress (), this);
	//	progress.setMinimumDuration (100);
	//	progress.setWindowModality(Qt::WindowModal);
	//
	//	connect (task, SIGNAL (progress (int)), &progress, SLOT (setValue (int)));
	//
	//	while (!task->isEnded ())
	//	{
	//		qApp->processEvents ();
	//		DefaultQThread::msleep (100);
	//
	//		if (progress.wasCanceled ())
	//		{
	//			task->cancel ();
	//			progress.setLabelText ("Cancel...");
	//		}
	//	}
}
