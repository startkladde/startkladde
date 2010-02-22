/*
 * AutomaticEntityList.h
 *
 *  Created on: Aug 31, 2009
 *      Author: Martin Herrmann
 */

#ifndef AUTOMATICENTITYLIST_H_
#define AUTOMATICENTITYLIST_H_

// TODO may includes in header
#include "EntityList.h"
#include "src/db/dataStorage/DataStorageMonitor.h"
#include "src/db/dataStorage/DataStorage.h"
#include "src/db/DbEvent.h"
#include "src/concurrent/threadUtil.h"

// ******************
// ** Construction **
// ******************

/**
 * A subclass auf EntityList that receives dbChange events from a DataStorage
 * and updates the list accordingly.
 */
template<class T> class AutomaticEntityList: public EntityList<T>, DataStorageMonitor::Listener
{
	public:
		AutomaticEntityList (DataStorage &dataStorage, QObject *parent=NULL);
		AutomaticEntityList (DataStorage &dataStorage, const QList<T> &list, QObject *parent=NULL);
		virtual ~AutomaticEntityList ();

		// DataStorageMonitor::Listener methods
		virtual void dbEvent (DbEvent event);

	protected:
		DataStorage &dataStorage;
		DataStorageMonitor monitor;
};

/**
 * Creates an empty AutomaticEntityList
 *
 * @param dataStorage the DataStorage to monitor for changes
 * @param parent the Qt parent
 */
template<class T> AutomaticEntityList<T>::AutomaticEntityList (DataStorage &dataStorage, QObject *parent):
	EntityList<T> (parent),
	dataStorage (dataStorage),
	monitor (dataStorage, *this)
{
}

/**
 * Creates an AutomaticEntityList with entries from a given list
 * @param dataStorage
 * @param list
 * @param parent
 * @return
 */
template<class T> AutomaticEntityList<T>::AutomaticEntityList (DataStorage &dataStorage, const QList<T> &list, QObject *parent):
	EntityList<T> (list, parent),
	dataStorage (dataStorage),
	monitor (dataStorage, *this)
{
}

template<class T> AutomaticEntityList<T>::~AutomaticEntityList ()
{
}


// ******************************************
// ** DataStorageMonitor::Listener methods **
// ******************************************

/**
 * Called on database changes. Updates the list and emits the appropriate
 * signals.
 *
 * @param event the DbEvent describing the change
 */
template<class T> void AutomaticEntityList<T>::dbEvent (DbEvent event)
{
	assert (isGuiThread());

	if (event.table!=DbEvent::tableAll && event.table!=DbEvent::getTable<T> ()) return;

	DataStorage &ds=/*EntityList<T>::*/dataStorage;

	switch (event.type)
	{
		case DbEvent::typeNone:
			break;
		case DbEvent::typeAdd:
		{
			EntityList<T>::append (ds.getObject<T> (event.id));
		} break;
		case DbEvent::typeDelete:
		{
			int i=EntityList<T>::findById (event.id);
			if (i>=0) EntityList<T>::removeAt (i);
		} break;
		case DbEvent::typeChange:
		{
			int i=EntityList<T>::findById (event.id);
			if (i>=0)
				EntityList<T>::replace (i, ds.getObject<T> (event.id));
			else
				// Should not happen
				EntityList<T>::append (ds.getObject<T> (event.id));
		} break;
		case DbEvent::typeRefresh:
		{
			EntityList<T>::replaceList (ds.getObjects<T> ());
		} break;
	}
}

#endif
