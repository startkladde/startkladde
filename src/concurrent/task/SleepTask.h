/*
 * SleepTask.h
 *
 *  Created on: Aug 9, 2009
 *      Author: mherrman
 */

#ifndef SLEEPTASK_H_
#define SLEEPTASK_H_

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
