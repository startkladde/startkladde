#include "OperationMonitorInterface.h"

#include <QAtomicInt>


#include "src/concurrent/monitor/OperationMonitor.h"

const OperationMonitorInterface OperationMonitorInterface::null (NULL);

OperationMonitorInterface::~OperationMonitorInterface ()
{
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
	// FIXME: this is very similar to QFutureInterfaceBase, but isn't there a
	// race condition in case the last instance is destroyed at this point?
	other.refCount->ref ();
}


OperationMonitorInterface &OperationMonitorInterface::operator= (const OperationMonitorInterface &other)
{
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

}

void OperationMonitorInterface::status (const QString &text, bool checkCanceled)
{
	if (monitor)
	{
		if (checkCanceled) this->checkCanceled ();
		monitor->setStatus (text);
	}
}

void OperationMonitorInterface::progress (int progress, int maxProgress, bool checkCanceled)
{
	if (monitor)
	{
		if (checkCanceled) this->checkCanceled ();
		monitor->setProgress (progress, maxProgress);
	}
}

/**
 * Can be called manuall, or is called automatically when only 1 reference is left.
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
		throw OperationMonitor::CanceledException ();
}
