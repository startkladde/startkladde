/*
 * DataStorageSleepTask.h
 *
 *  Created on: Aug 15, 2009
 *      Author: Martin Herrmann
 */

#ifndef _DataStorageTask_h
#define _DataStorageTask_h

#include "src/concurrent/task/Task.h"

class DataStorage;

class DataStorageSleepTask: public Task
{
	public:
		DataStorageSleepTask (DataStorage &dataStorage, int seconds);
		virtual ~DataStorageSleepTask ();

		virtual QString toString () const;
		virtual bool run ();

	private:
		DataStorage &dataStorage;
		int seconds;
};

#endif /* DATASTORAGESLEEPTASK_H_ */
