#ifndef TASKPROGRESSDIALOG_H
#define TASKPROGRESSDIALOG_H

#include <QtGui/QDialog>

#include "ui_TaskProgressDialog.h"

class Task;

/*
 * TODO: between Task::status emitting "statusChanged" and this dialog
 * receiving "status", there's some time.
 */


/**
 * A dialog for displaying the status of a Task
 *
 * Example usage:
 *   Task *task=new SleepTask (5);
 *   dataStorage.addTask (task);
 *   TaskProgressDialog::waitTask (this, task);
 *   delete task;
 */
class TaskProgressDialog: public QDialog
{
    Q_OBJECT

	public:
		/**
		 * Waits for the task to end, showing a modal dialog. If the task
		 * already ended, returns immediately. If the user presses cancel,
		 * the task is canceled. This function returns only after the task
		 * has ended, so the task can be deleted as soon as this method
		 * returns (unless it is in use otherwise).
		 */
		static void waitTask (QWidget *parent, Task *task);

	public slots:
		virtual void reject ();

	protected:
		TaskProgressDialog (QWidget *parent, Task *task);
		~TaskProgressDialog();

	protected slots:
		void progress (int progress, int maxProgress);
		void status (QString status);

	private:
		Ui::TaskProgressDialogClass ui;

		Task *task;
};

#endif // TASKPROGRESSDIALOG_H
