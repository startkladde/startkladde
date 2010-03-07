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

class QWidget;

/**
 * Contains the database related objects required in the GUI:
 *   - the Interface
 *   - the Database (ORM)
 *   - the Cache
 * as well as various workers and some functionality related to database
 * management.
 *
 * One advantage to this class is that the workers can be reused; e. g. we
 * don't have to create DbWorker (and the associated thread) each time we want
 * to add or update an object.
 *
 * This class is quite heavyweight, so care should be taken in making other
 * classes depend on it. As a rule, non-GUI classes should not use a DbManager,
 * unless there is good reason.
 */
class DbManager
{
	public:
		class ConnectCanceledException {};

		class ConnectFailedException
		{
			public:
				ConnectFailedException (const QString &message): message (message) {}
				QString message;
		};


		DbManager (const DatabaseInfo &info);
		virtual ~DbManager ();

		virtual Db::Interface::ThreadSafeInterface
		                               &getInterface          () { return interface;   }
		virtual Db::Database           &getDb                 () { return db;          }
		virtual Db::Cache::Cache       &getCache              () { return cache;       }

		virtual Db::DbWorker           &getDbWorker           () { return dbWorker;    }
		virtual Db::Migration::Background::BackgroundMigrator
		                               &getBackgroundMigrator () { return migrator;    }
		virtual Db::Cache::CacheThread &getCacheThread        () { return cacheThread; }


		// *** Schema management
		bool isCurrent (QWidget *parent);
		void ensureCurrent (const QString &message, QWidget *parent);
		bool isEmpty (QWidget *parent);


		// *** Connection management


		// *** Data management
		void clearCache ();
		void refreshCache (QWidget *parent);
		void fetchFlights (QDate date, QWidget *parent);


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
