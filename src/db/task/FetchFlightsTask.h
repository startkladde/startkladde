/*
 * FetchFlightsTask.h
 *
 *  Created on: Sep 6, 2009
 *      Author: deffi
 */

#ifndef FETCHFLIGHTSTASK_H_
#define FETCHFLIGHTSTASK_H_

#include <QDate>

// TODO reduce dependencies
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

#endif /* FETCHFLIGHTSTASK_H_ */
