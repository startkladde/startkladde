/*
 * RefreshAllTask.h
 *
 *  Created on: Aug 16, 2009
 *      Author: Martin Herrmann
 */

#ifndef _RefreshAllTask_h
#define _RefreshAllTask_h

#include "src/concurrent/task/Task.h"

class DataStorage;

class RefreshAllTask: public Task
{
	public:
		RefreshAllTask (DataStorage &dataStorage);
		virtual ~RefreshAllTask ();

		virtual QString toString () const;
		virtual bool run ();

	private:
		DataStorage &dataStorage;
};

#endif /* REFRESHALLTASK_H_ */
