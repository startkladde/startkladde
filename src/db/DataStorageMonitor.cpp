/*
 * DataStorageMonitor.cpp
 *
 *  Created on: Aug 29, 2009
 *      Author: mherrman
 */

#include "DataStorageMonitor.h"

#include "DataStorage.h"

DataStorageMonitor::DataStorageMonitor (DataStorage &dataStorage, DataStorageMonitor::Listener &listener):
	listener (listener)
{
	QObject::connect (&dataStorage, SIGNAL (dbEvent (DbEvent)), this, SLOT (dbEvent (DbEvent)));
}

DataStorageMonitor::~DataStorageMonitor ()
{
}

void DataStorageMonitor::dbEvent (DbEvent event)
{
	listener.dbEvent (event);
}
