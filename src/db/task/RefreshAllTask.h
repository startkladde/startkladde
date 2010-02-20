/*
 * RefreshAllTask.h
 *
 *  Created on: Aug 16, 2009
 *      Author: mherrman
 */

#ifndef REFRESHALLTASK_H_
#define REFRESHALLTASK_H_

// TODO reduce dependencies
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
