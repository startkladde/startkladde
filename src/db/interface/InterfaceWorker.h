/*
 * InterfaceWorker.h
 *
 *  Created on: 08.03.2010
 *      Author: Martin Herrmann
 */

#ifndef INTERFACEWORKER_H_
#define INTERFACEWORKER_H_

#include <QObject>
#include <QThread>

template<typename T> class Returner;
class OperationMonitor;
class ThreadSafeInterface;

class InterfaceWorker: public QObject
{
	Q_OBJECT

	public:
		InterfaceWorker (ThreadSafeInterface &interface);
		virtual ~InterfaceWorker ();

		virtual ThreadSafeInterface &getInterface () { return interface; }

		virtual void open           (Returner<bool> &returner, OperationMonitor &monitor);
		virtual void createDatabase (Returner<void> &returner, OperationMonitor &monitor, const QString &name, bool skipIfExists=false);
    	virtual void grantAll       (Returner<void> &returner, OperationMonitor &monitor, const QString &database, const QString &username, const QString &password="");

	signals:
		virtual void sig_open           (Returner<bool> *returner, OperationMonitor *monitor);
		virtual void sig_createDatabase (Returner<void> *returner, OperationMonitor *monitor, QString name, bool skipIfExists);
    	virtual void sig_grantAll       (Returner<void> *returner, OperationMonitor *monitor, QString database, QString username, QString password);

	protected slots:
		virtual void slot_open           (Returner<bool> *returner, OperationMonitor *monitor);
		virtual void slot_createDatabase (Returner<void> *returner, OperationMonitor *monitor, QString name, bool skipIfExists);
    	virtual void slot_grantAll       (Returner<void> *returner, OperationMonitor *monitor, QString database, QString username, QString password);

	private:
		QThread thread;
		ThreadSafeInterface &interface;
};

#endif
