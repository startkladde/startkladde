/*
 * UpdateobjectTask.h
 *
 *  Created on: Aug 15, 2009
 *      Author: Martin Herrmann
 */

#ifndef UPDATEOBJECTTASK_H_
#define UPDATEOBJECTTASK_H_

// TODO many tasks depend on DataStorage, which is bad because everything that
// depends on the task will have to be rebuilt when DataStorage.h is changed
// (only applies to template classes with the implementation in the header).
#include "src/db/DataStorage.h"

template<class T> class UpdateObjectTask: public Task
{
	public:
		UpdateObjectTask (DataStorage &dataStorage, const T &object):
			dataStorage (dataStorage), object (object)
		{
		}

		virtual QString toString () const
		{
			return QString::fromUtf8 ("%1 in Datenbank aktualisieren").arg (T::objectTypeDescription ());
		}

		virtual bool run ()
		{
			bool success;
			QString message;
			bool completed=dataStorage.updateObject (this, object, &success, &message);
			setSuccess (success);
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
