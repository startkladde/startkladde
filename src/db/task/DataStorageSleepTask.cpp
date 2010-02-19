/*
 * DataStorageSleepTask.cpp
 *
 *  Created on: Aug 15, 2009
 *      Author: mherrman
 */

#include "DataStorageSleepTask.h"

DataStorageSleepTask::DataStorageSleepTask (DataStorage &dataStorage, int seconds):
	dataStorage (dataStorage), seconds (seconds)
{
}

DataStorageSleepTask::~DataStorageSleepTask ()
{
}

QString DataStorageSleepTask::toString () const
{
	return "Warten (DataStorage)";
}

bool DataStorageSleepTask::run ()
{
	return dataStorage.sleep (*this, seconds);
}
