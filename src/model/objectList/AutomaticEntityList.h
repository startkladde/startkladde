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
#include "src/db/cache/Cache.h"
#include "src/db/event/Event.h"
#include "src/db/event/Monitor.h"
#include "src/concurrent/threadUtil.h"

// ******************
// ** Construction **
// ******************

/**
 * A subclass auf EntityList that receives dbChange events from a Cache
 * and updates the list accordingly.
 */
template<class T> class AutomaticEntityList: public EntityList<T>, Db::Event::Monitor::Listener
{
	public:
		AutomaticEntityList (Db::Cache::Cache &cache, QObject *parent=NULL);
		AutomaticEntityList (Db::Cache::Cache &cache, const QList<T> &list, QObject *parent=NULL);
		virtual ~AutomaticEntityList ();

		// Db::Event::Listener methods
		virtual void dbEvent (Db::Event::Event event);

	protected:
		Db::Cache::Cache &cache;
		Db::Event::Monitor monitor;
};

/**
 * Creates an empty AutomaticEntityList
 *
 * @param cache the Db::Cache::Cache to monitor for changes
 * @param parent the Qt parent
 */
template<class T> AutomaticEntityList<T>::AutomaticEntityList (Db::Cache::Cache &cache, QObject *parent):
	EntityList<T> (parent),
	cache (cache),
	monitor (cache, SIGNAL (changed (Db::Event::Event)), *this)
{
}

/**
 * Creates an AutomaticEntityList with entries from a given list
 * @param cache
 * @param list
 * @param parent
 * @return
 */
template<class T> AutomaticEntityList<T>::AutomaticEntityList (Db::Cache::Cache &cache, const QList<T> &list, QObject *parent):
	EntityList<T> (list, parent),
	cache (cache),
	monitor (cache, SIGNAL (changed (Db::Event::Event)), *this)
{
}

template<class T> AutomaticEntityList<T>::~AutomaticEntityList ()
{
}


// *********************************
// ** Db::Event::Listener methods **
// *********************************

/**
 * Called on database changes. Updates the list and emits the appropriate
 * signals.
 *
 * @param event the Db::Event::Event describing the change
 */
template<class T> void AutomaticEntityList<T>::dbEvent (Db::Event::Event event)
{
	assert (isGuiThread());

	// Return if the table does not match the type of this list
	if (event.table!=Db::Event::Event::getTable<T> ()) return;

	switch (event.type)
	{
		case Db::Event::Event::typeAdd:
		{
			EntityList<T>::append (cache.getObject<T> (event.id));
		} break;
		case Db::Event::Event::typeDelete:
		{
			int i=EntityList<T>::findById (event.id);
			if (i>=0) EntityList<T>::removeAt (i);
		} break;
		case Db::Event::Event::typeChange:
		{
			int i=EntityList<T>::findById (event.id);
			if (i>=0)
				EntityList<T>::replace (i, cache.getObject<T> (event.id));
			else
				// Should not happen
				EntityList<T>::append (cache.getObject<T> (event.id));
		} break;
	}
}

#endif
