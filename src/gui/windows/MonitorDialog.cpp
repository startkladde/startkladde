#include "MonitorDialog.h"

#include <iostream> //remove

// FIXME synchronization

#include "src/concurrent/monitor/SignalOperationMonitor.h"

MonitorDialog::MonitorDialog (SignalOperationMonitor &monitor, QWidget *parent):
	QDialog (parent), theMonitor (monitor)
{
	ui.setupUi (this);

//	setModal (true);

	QObject::connect (&theMonitor, SIGNAL (ended ())                  , this, SLOT (accept ()          ));
	QObject::connect (&theMonitor, SIGNAL (progressChanged (int, int)), this, SLOT (progress (int, int)));
	QObject::connect (&theMonitor, SIGNAL (statusChanged (QString))   , this, SLOT (status (QString)   ));

	QObject::connect (ui.cancelButton, SIGNAL (clicked ()), this, SLOT (reject ()));
}

MonitorDialog::~MonitorDialog()
{

}

void MonitorDialog::monitor (SignalOperationMonitor &monitor, QWidget *parent)
{
	MonitorDialog dialog (monitor, parent);

	// Check after the signals have been connected
	if (monitor.getEnded ())
		return;

	dialog.exec ();
}

void MonitorDialog::reject ()
{
	ui.statusLabel->setText ("Abbrechen...");
	theMonitor.cancel ();
}

void MonitorDialog::progress (int progress, int maxProgress)
{
	ui.progressBar->setValue (progress);
	if (maxProgress>=0)
		ui.progressBar->setMaximum (maxProgress);

	if (progress==maxProgress && progress>0)
		accept ();
}

void MonitorDialog::status (QString status)
{
	ui.statusLabel->setText (status);
}

