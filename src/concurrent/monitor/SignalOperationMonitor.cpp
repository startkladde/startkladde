#include "SignalOperationMonitor.h"

#include <iostream>

SignalOperationMonitor::SignalOperationMonitor ()
{
}

SignalOperationMonitor::~SignalOperationMonitor ()
{
}

void SignalOperationMonitor::cancel ()
{
	OperationMonitor::cancel ();
}

void SignalOperationMonitor::setStatus (const QString &text)
{
	emit statusChanged (text);
}

void SignalOperationMonitor::setProgress (int progress, int maxProgress)
{
	emit progressChanged (progress, maxProgress);
}

void SignalOperationMonitor::setEnded ()
{
	emit ended ();
}
