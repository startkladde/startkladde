/*
 * DbManager.h
 *
 *  Created on: 07.03.2010
 *      Author: Martin Herrmann
 */

#ifndef DBMANAGER_H_
#define DBMANAGER_H_

#include <cassert>

#include "src/db/DatabaseInfo.h"
#include "src/db/interface/threadSafe/ThreadSafeInterface.h"
#include "src/db/Database.h"
#include "src/db/cache/Cache.h"
#include "src/db/DbWorker.h"
#include "src/db/migration/background/BackgroundMigrator.h"
#include "src/db/cache/CacheThread.h"


/**
 * Contains the database related objects required in the GUI:
 *   - the Interface
 *   - the Database (ORM)
 *   - the Cache
 * as well as various workers.
 */
class DbManager
{
	public:
		DbManager (const DatabaseInfo &info);
		virtual ~DbManager ();

		virtual Db::Interface::ThreadSafeInterface &getInterface () { return interface; }
		virtual Db::Database                       &getDb        () { return db;        }
		virtual Db::Cache::Cache                   &getCache     () { return cache;     }

		virtual Db::DbWorker                       &getDbWorker  () { return dbWorker;  }
		virtual Db::Migration::Background::BackgroundMigrator &getBackgroundMigrator () { return migrator; }
		virtual Db::Cache::CacheThread             &getCacheThread () { return cacheThread; }

	private:
		DbManager (const DbManager &other);
		DbManager &operator= (const DbManager &other);

		Db::Interface::ThreadSafeInterface interface;
		Db::Database db;
		Db::Cache::Cache cache;

		Db::DbWorker dbWorker;
		Db::Migration::Background::BackgroundMigrator migrator;
		Db::Cache::CacheThread cacheThread;

};

#endif
