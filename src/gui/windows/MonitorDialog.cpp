#include "MonitorDialog.h"

// FIXME synchronization
// FIXME must handle cancel, finished (other than by progress==100%)

#include "src/concurrent/monitor/SignalOperationMonitor.h"

MonitorDialog::MonitorDialog (SignalOperationMonitor &monitor, QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);

//	setModal (true);

//	QObject::connect (&monitor, SIGNAL (ended ())                  , this, SLOT (accept ()          ));
	QObject::connect (&monitor, SIGNAL (progressChanged (int, int)), this, SLOT (progress (int, int)));
	QObject::connect (&monitor, SIGNAL (statusChanged (QString))   , this, SLOT (status (QString)   ));
}

MonitorDialog::~MonitorDialog()
{

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

