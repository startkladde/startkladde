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

	public slots:
		virtual void cancel ();

	signals:
		void statusChanged (QString text);
		void progressChanged (int progress, int maxProgress);
		void ended ();

	private:
		virtual void setStatus (const QString &text);
		virtual void setProgress (int progress, int maxProgress);
		virtual void setEnded ();
};

#endif
