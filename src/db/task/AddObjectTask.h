/*
 * AddObjectTask.h
 *
 *  Created on: Aug 15, 2009
 *      Author: Martin Herrmann
 */

#ifndef _AddObjectTask_h
#define _AddObjectTask_h

#include "src/concurrent/task/Task.h"
#include "src/db/DataStorage.h"

template<class T> class AddObjectTask: public Task
{
	public:
		AddObjectTask (DataStorage &dataStorage, const T &object):
			dataStorage (dataStorage), object (object)
		{
		}

		virtual QString toString () const
		{
			return QString::fromUtf8 ("%1 zur Datenbank dazufügen").arg (T::objectTypeDescription ());
		}

		virtual bool run ()
		{
			bool succ;
			QString message;
			bool completed=dataStorage.addObject (this, object, &id, &succ, &message);
			setSuccess (succ);
			setMessage (message);
			return completed;
		}

		virtual db_id getId () const { return id; }

	private:
		DataStorage &dataStorage;
		const T &object;
		db_id id;
};

#endif
