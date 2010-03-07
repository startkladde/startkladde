#include "DbManager.h"

DbManager::DbManager (const DatabaseInfo &info):
	interface (info), db (interface), cache (db),
	dbWorker (db), migrator (interface), cacheThread (cache)
{
}

DbManager::DbManager (const DbManager &other):
	interface (other.interface.getInfo ()), db (interface), cache (db),
	dbWorker (db), migrator (interface), cacheThread (cache)
{
	assert (!"DbManager copied");
}

DbManager &DbManager::operator= (const DbManager &other)
{
	(void)other;
	assert (!"DbManager copied");
}

DbManager::~DbManager ()
{
}
