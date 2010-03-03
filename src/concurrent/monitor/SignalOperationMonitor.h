/*
 * SignalOperationMonitor.h
 *
 *  Created on: 03.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SIGNALOPERATIONMONITOR_H_
#define SIGNALOPERATIONMONITOR_H_

#include <QObject>

#include "OperationMonitor.h"

class SignalOperationMonitor: public QObject, public OperationMonitor
{
	Q_OBJECT

	public:

		SignalOperationMonitor ();
		virtual ~SignalOperationMonitor ();

		virtual bool isCanceled () const;

		virtual void status (QString text);
		virtual void progress (int progress, int maxProgress);

	signals:
		void statusChanged (QString text);
		void progressChanged (int progress, int maxProgress);
};

#endif
