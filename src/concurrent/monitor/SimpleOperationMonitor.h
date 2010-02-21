/*
 * SimpleOperationMonitor.h
 *
 *  Created on: Aug 2, 2009
 *      Author: Martin Herrmann
 */

#ifndef SIMPLEOPERATIONMONITOR_H_
#define SIMPLEOPERATIONMONITOR_H_

#include "OperationMonitor.h"

class SimpleOperationMonitor: public OperationMonitor
{
	public:
		SimpleOperationMonitor ();
		virtual ~SimpleOperationMonitor ();

		virtual bool isCanceled () const;

		virtual void status (QString text);
		virtual void progress (int progress, int maxProgress);
};

#endif
