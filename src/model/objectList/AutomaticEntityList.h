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
#include "src/db/event/DbEvent.h"
#include "src/db/event/DbEventMonitor.h"
#include "src/concurrent/threadUtil.h"

// ******************
// ** Construction **
// ******************

/**
 * A subclass auf EntityList that receives changed events from an object
 * and updates the list accordingly
 *
 * The object must provide a changed (Db::Event::DbEvent) signal.
 *
 * This uses a DbEventMonitor rather than an AutomaticEntityListBase with a
 * changed slot in order to avoid a diamon inheritance from QObject.
 */
template<class T> class AutomaticEntityList: public EntityList<T>, Db::Event::DbEventMonitor::Listener
{
	public:
		AutomaticEntityList (QObject &source, QObject *parent=NULL);
		AutomaticEntityList (QObject &source, const QList<T> &list, QObject *parent=NULL);
		virtual ~AutomaticEntityList ();

		// Db::Event::Listener methods
		virtual void dbEvent (Db::Event::DbEvent event);

	protected:
		Db::Event::DbEventMonitor monitor;
};

/**
 * Creates an empty AutomaticEntityList
 *
 * @param source the object to monitor for changes
 * @param parent the Qt parent
 */
template<class T> AutomaticEntityList<T>::AutomaticEntityList (QObject &source, QObject *parent):
	EntityList<T> (parent),
	monitor (source, SIGNAL (changed (Db::Event::DbEvent)), *this)
{
}

/**
 * Creates an AutomaticEntityList with entries from a given list
 * @param source
 * @param list
 * @param parent
 * @return
 */
template<class T> AutomaticEntityList<T>::AutomaticEntityList (QObject &source, const QList<T> &list, QObject *parent):
	EntityList<T> (list, parent),
	monitor (source, SIGNAL (changed (Db::Event::DbEvent)), *this)
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
 * @param event the Db::Event::DbEvent describing the change
 */
template<class T> void AutomaticEntityList<T>::dbEvent (Db::Event::DbEvent event)
{
	assert (isGuiThread());

	// Return if the table does not match the type of this list
	if (!event.hasTable<T> ()) return;

	switch (event.getType ())
	{
		case Db::Event::DbEvent::typeAdd:
		{
			EntityList<T>::append (event.getValue<T> ());
		} break;
		case Db::Event::DbEvent::typeDelete:
		{
			int i=EntityList<T>::findById (event.getId ());
			if (i>=0) EntityList<T>::removeAt (i);
		} break;
		case Db::Event::DbEvent::typeChange:
		{
			int i=EntityList<T>::findById (event.getId ());
			if (i>=0)
				EntityList<T>::replace (i, event.getValue<T> ());
			else
				// Should not happen
				EntityList<T>::append (event.getValue<T> ());
		} break;
	}
}

#endif
