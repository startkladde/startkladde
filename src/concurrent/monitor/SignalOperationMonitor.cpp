#include "SignalOperationMonitor.h"

#include <iostream>

#include "src/util/qString.h"

SignalOperationMonitor::SignalOperationMonitor ():
	hasEnded (false)
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
	std::cout << "status: " << text << std::endl;

	status=text;
	emit statusChanged (text);
}

void SignalOperationMonitor::setProgress (int progress, int maxProgress)
{
	emit progressChanged (progress, maxProgress);
}

void SignalOperationMonitor::setEnded ()
{
	hasEnded=true;
	emit ended ();
}

bool SignalOperationMonitor::getEnded () const
{
	return hasEnded;
}

const QString &SignalOperationMonitor::getStatus () const
{
	return status;
}
