#include "MonitorDialog.h"

#include <iostream> //remove

#include "src/util/qString.h"

// FIXME synchronization
//  - for example, the first status may not be displayed if it was set before the dialog was displayed

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
	// It seems like we can get an erroneous "1%" indication if we set the
	// value before the maximum.
	if (maxProgress>=0)
		ui.progressBar->setMaximum (maxProgress);
	ui.progressBar->setValue (progress);
}

void MonitorDialog::status (QString status)
{
	std::cout << "status " << status << std::endl;

	ui.statusLabel->setText (status);
}

