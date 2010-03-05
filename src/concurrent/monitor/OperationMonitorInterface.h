/*
 * OperationMonitorInterface.h
 *
 *  Created on: 04.03.2010
 *      Author: Martin Herrmann
 */

#ifndef OPERATIONMONITORINTERFACE_H_
#define OPERATIONMONITORINTERFACE_H_

#include <QString>

class QAtomicInt;
class OperationMonitor;


class OperationMonitorInterface
{
	friend class OperationMonitor;

	public:
		// ** Constants
		static const OperationMonitorInterface null;

		// ** Construction
		~OperationMonitorInterface ();
		OperationMonitorInterface (const OperationMonitorInterface &other);
		OperationMonitorInterface &operator= (const OperationMonitorInterface &other);

		// Operation feedback
		void status (const QString &text, bool checkCanceled=true);
		void status (const char *text, bool checkCanceled=true);
		void progress (int progress, int maxProgress, const QString &status=QString (), bool checkCanceled=true);
		void progress (int progress, int maxProgress, const char *status, bool checkCanceled=true);
		void ended ();

		// Operation control
		bool canceled ();
		void checkCanceled ();

	private:
		OperationMonitorInterface (OperationMonitor *monitor);
		OperationMonitor *monitor;
		QAtomicInt *refCount;
};

#endif
