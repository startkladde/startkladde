#include "OperationMonitorInterface.h"

#include <iostream>

#include <QAtomicInt>

#include "src/util/qString.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationCanceledException.h"

const OperationMonitorInterface OperationMonitorInterface::null (NULL);

OperationMonitorInterface::~OperationMonitorInterface ()
{
//	std::cout << "-interface" << std::endl;

	int newRefCount=refCount->fetchAndAddOrdered (-1)-1;

	if (newRefCount==1)
	{
		if (monitor)
			monitor->setEnded ();
	}
	else if (newRefCount==0)
	{
		delete refCount;
	}
}

OperationMonitorInterface::OperationMonitorInterface (const OperationMonitorInterface &other):
	monitor (other.monitor), refCount (other.refCount)
{
//	std::cout << "+interface" << std::endl;

	// FIXME: this is very similar to QFutureInterfaceBase, but isn't there a
	// race condition in case the last instance is destroyed at this point?
	other.refCount->ref ();
}


OperationMonitorInterface &OperationMonitorInterface::operator= (const OperationMonitorInterface &other)
{
//	std::cout << "=interface" << std::endl;

	if (&other==this) return *this;

	// FIXME: this is very similar to QFutureInterfaceBase, but isn't there a
	// race condition in case the last instance is destroyed at this point?
	other.refCount->ref ();

	int newRefCount=refCount->fetchAndAddOrdered (-1)-1;

	if (newRefCount==1)
		monitor->setEnded ();
	else if (newRefCount==0)
		delete refCount;

	refCount=other.refCount;
	monitor=other.monitor;

	return *this;
}

OperationMonitorInterface::OperationMonitorInterface (OperationMonitor *monitor):
	monitor (monitor), refCount (new QAtomicInt (1))
{
//	std::cout << "+interface" << std::endl;
}

void OperationMonitorInterface::status (const QString &text, bool checkCanceled)
{
	if (monitor)
	{
		if (checkCanceled) this->checkCanceled ();

		monitor->setStatus (text);
	}
}

void OperationMonitorInterface::status (const char *text, bool checkCanceled)
{
	status (utf8 (text), checkCanceled);
}

void OperationMonitorInterface::progress (int progress, int maxProgress, const QString &status, bool checkCanceled)
{
	if (monitor)
	{
		if (checkCanceled) this->checkCanceled ();
		monitor->setProgress (progress, maxProgress);
		if (!status.isNull ()) monitor->setStatus (status);
	}
}

void OperationMonitorInterface::progress (int progress, int maxProgress, const char *status, bool checkCanceled)
{
	if (status)
		this->progress (progress, maxProgress, utf8 (status), checkCanceled);
	else
		this->progress (progress, maxProgress, QString (), checkCanceled);
}

/**
 * Can be called manually, or is called automatically when only 1 reference is left.
 */
void OperationMonitorInterface::ended ()
{
	if (monitor)
		monitor->setEnded ();
}

bool OperationMonitorInterface::canceled ()
{
	if (monitor)
		return monitor->isCanceled ();
	else
		return false;
}

void OperationMonitorInterface::checkCanceled ()
{
	if (canceled ())
		throw OperationCanceledException ();
}
