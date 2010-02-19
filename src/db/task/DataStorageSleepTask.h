/*
 * DataStorageSleepTask.h
 *
 *  Created on: Aug 15, 2009
 *      Author: mherrman
 */

#ifndef DATASTORAGESLEEPTASK_H_
#define DATASTORAGESLEEPTASK_H_

#include "src/concurrent/task/Task.h"
#include "src/db/DataStorage.h"

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
