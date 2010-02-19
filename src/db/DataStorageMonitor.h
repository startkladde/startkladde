/*
 * DataStorageMonitor.h
 *
 *  Created on: Aug 29, 2009
 *      Author: mherrman
 */

#ifndef DATASTORAGEMONITOR_H_
#define DATASTORAGEMONITOR_H_

#include <QObject>

#include <src/db/DbEvent.h>

class DataStorage;

/**
 * Implements the listener pattern for DbEvents emitted by a DataStorage.
 *
 * This is useful for template classes which cannot be QObjects and thus
 * cannot define slots. An alternative solution may be to use a QObject base
 * class to the template class, but that usually involves diamond inheritance.
 */
class DataStorageMonitor: public QObject
{
	Q_OBJECT

	public:
		// Types
		class Listener
		{
			public:
				virtual void dbEvent (DbEvent event)=0;
		};

		// Construction
		DataStorageMonitor (DataStorage &dataStorage, Listener &listener);
		virtual ~DataStorageMonitor ();

	public slots:
		virtual void dbEvent (DbEvent event);

	private:
		Listener &listener;
};

#endif /* DATASTORAGEMONITOR_H_ */
