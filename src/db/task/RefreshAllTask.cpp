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
