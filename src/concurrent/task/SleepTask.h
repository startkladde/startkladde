/*
 * SleepTask.h
 *
 *  Created on: Aug 9, 2009
 *      Author: Martin Herrmann
 */

#ifndef _SleepTask_h
#define _SleepTask_h

#include "Task.h"

class SleepTask: public Task
{
	public:
		SleepTask (int seconds);
		virtual ~SleepTask ();

		QString toString () const;

	protected:
		bool run ();

	private:
		int seconds;
};

#endif /* SLEEPTASK_H_ */
