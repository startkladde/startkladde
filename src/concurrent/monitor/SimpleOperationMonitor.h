/*
 * SimpleOperationMonitor.h
 *
 *  Created on: Aug 2, 2009
 *      Author: Martin Herrmann
 */

#ifndef _SimpleOperationMonitor_h
#define _SimpleOperationMonitor_h

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
