/*
 * OperationMonitor.h
 *
 *  Created on: Aug 2, 2009
 *      Author: mherrman
 */

#ifndef _OperationMonitor_h
#define _OperationMonitor_h

#include <QString>

/**
 * A abstract class for communicating with a function that may take some time
 *
 * An OperationMonitor can be passed into such a function. The function can
 * query the monitor for whether the operation should be canceled and use the
 * monitor to emit information about the status/progress.
 */
class OperationMonitor
{
	public:
		OperationMonitor ();
		virtual ~OperationMonitor ();

		// ***** To task

		/** @brief returns true if the operation should be canceled */
		virtual bool isCanceled () const=0;

		// ***** From task

		/** @brief what we are currently doing */
		virtual void status (QString text)=0;
		virtual void status (const char *text) { status (QString::fromUtf8 (text)); }

		/** @brief the progress of the operation */
		virtual void progress (int progress, int maxProgress)=0;
};

#endif
