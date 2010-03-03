#include "SignalOperationMonitor.h"

SignalOperationMonitor::SignalOperationMonitor ()
{
}

SignalOperationMonitor::~SignalOperationMonitor ()
{
}

bool SignalOperationMonitor::isCanceled () const
{
	// FIXME implement canceling
	return false;
}

void SignalOperationMonitor::status (QString text)
{
	emit statusChanged (text);
}

void SignalOperationMonitor::progress (int progress, int maxProgress)
{
	emit progressChanged (progress, maxProgress);
}
