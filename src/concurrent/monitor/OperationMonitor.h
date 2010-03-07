/*
 * OperationMonitor.h
 *
 *  Created on: Aug 2, 2009
 *      Author: Martin Herrmann
 */

#ifndef OPERATIONMONITOR_H_
#define OPERATIONMONITOR_H_

#include <QString>
#include <QMutex>

#include "src/StorableException.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"

class QAtomicInt;


/**
 * A class that allows monitoring and canceling an operation, typically running
 * in a different task
 *
 * The operation accesses the task through its OperationMonitorInterface.
 *
 * How to use:
 *   asyncOperation (Returner<T> *returner, OperationMonitor *monitor)
 *   {
 * 		returnOrException (returner, actualOperation (monitor->interface ()));
 *   }
 * The interface is passed by copy. After the last copy is destroyed, the end
 * of the operation is signaled to the monitor, as if interface.ended () had
 * been called.
 *
 * When using with an operation that does not take an OperationMonitorInterface,
 * it still has to be fetched and destroyed at the correct time to signal the
 * end of the operation:
 *   asyncOperation (Returner<T> *returner, OperationMonitor *monitor)
 *   {
 * 		OperationMonitorInterface interface=monitor->interface ();
 * 		returnOrException (returner, actualOperation);
 *   }
 * See, for example, ThreadSafeInterface::asyncOpen.
 */
class OperationMonitor
{
	public:
		friend class OperationMonitorInterface;

		// ** Construction
		OperationMonitor ();
		virtual ~OperationMonitor ();

		// ** Getting the interface
		virtual OperationMonitorInterface interface ();
		virtual operator OperationMonitorInterface ();

		// ** Operation control
		/** Signals the operation to cancel */
		virtual void cancel ();

	private:
		/** The master copy of this monitor's interface */
		OperationMonitorInterface theInterface;

		bool canceled;
		mutable QMutex mutex;

		// ** Operation feedback
		virtual void setStatus (const QString &text)=0;
		virtual void setProgress (int progress, int maxProgress)=0;

		/** Signals that the operation ended (canceled or finished) */
		virtual void setEnded ()=0;

		// ** Operation control
		virtual bool isCanceled ();
};

#endif
