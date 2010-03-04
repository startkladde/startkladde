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
 * The operation accesses the task through its interface.
 */
class OperationMonitor
{
	public:
		friend class OperationMonitorInterface;

		class CanceledException: public StorableException
		{
			virtual CanceledException *clone   () const { return new CanceledException (); }
			virtual void               rethrow () const { throw      CanceledException (); }
		};


		// ** Construction
		OperationMonitor ();
		virtual ~OperationMonitor ();

		// ** Getting the interface
		virtual OperationMonitorInterface interface ();
		//virtual operator Interface ();

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
