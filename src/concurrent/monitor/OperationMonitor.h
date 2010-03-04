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
		class Interface
		{
			friend class OperationMonitor;
			public:
				~Interface ();
				Interface (const Interface &other);
				Interface &operator= (const Interface &other);

				// Operation feedback
				void status (const QString &text);
				void progress (int progress, int maxProgress);
				void ended ();

				// Operation control
				bool canceled ();

			private:
				Interface (OperationMonitor *monitor);
				OperationMonitor *monitor;
				QAtomicInt *refCount;
		};

		friend class Interface;

		// ** Construction
		OperationMonitor ();
		virtual ~OperationMonitor ();

		// ** Getting the interface
		virtual Interface interface ();
		//virtual operator Interface ();

		// ** Operation control
		/** Signals the operation to cancel */
		virtual void cancel ();

	private:
		/** The master copy of this monitor's interface */
		Interface theInterface;

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
