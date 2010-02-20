/*
 * FetchFlightsTask.h
 *
 *  Created on: Sep 6, 2009
 *      Author: Martin Herrmann
 */

#ifndef _FetchFlightsTask_h
#define _FetchFlightsTask_h

#include <QDate>

#include "src/concurrent/task/Task.h"

class DataStorage;

class FetchFlightsTask: public Task
{
	public:
		FetchFlightsTask (DataStorage &dataStorage, QDate date);
		virtual ~FetchFlightsTask ();

		virtual QString toString () const;
		virtual bool run ();

	private:
		DataStorage &dataStorage;
		QDate date;
};

#endif
