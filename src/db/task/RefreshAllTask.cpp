/*
 * RefreshAllTask.cpp
 *
 *  Created on: Aug 16, 2009
 *      Author: mherrman
 */

#include "RefreshAllTask.h"

#include "src/db/DataStorage.h"

RefreshAllTask::RefreshAllTask (DataStorage &dataStorage):
	dataStorage (dataStorage)
{
}

RefreshAllTask::~RefreshAllTask ()
{
}

QString RefreshAllTask::toString () const
{
	return "Alles aktualisieren";
}

bool RefreshAllTask::run ()
{
	return dataStorage.refreshAll (this);
}
