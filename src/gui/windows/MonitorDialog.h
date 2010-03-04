#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include <QtGui/QDialog>
#include "ui_MonitorDialog.h"

class SignalOperationMonitor;

class MonitorDialog: public QDialog
{
    Q_OBJECT

	public:
		MonitorDialog (SignalOperationMonitor &monitor, QWidget *parent=NULL);
		~MonitorDialog ();
		static void monitor (SignalOperationMonitor &monitor, QWidget *parent=NULL);

	protected slots:
		void progress (int progress, int maxProgress);
		void status (QString status);
		virtual void reject ();

	private:
		SignalOperationMonitor &theMonitor;
		Ui::MonitorDialogClass ui;
};

#endif // MONITORDIALOG_H
