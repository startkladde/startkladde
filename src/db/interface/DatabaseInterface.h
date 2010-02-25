/*
 * DatabaseInterface.h
 *
 *  Created on: 22.02.2010
 *      Author: Martin Herrmann
 */

#include <QString>
#include <QStringList>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "src/db/DatabaseInfo.h"

#ifndef DATABASEINTERFACE_H_
#define DATABASEINTERFACE_H_

namespace Db
{
	namespace Interface
	{
		/**
		 * Encapsulates a QSqlDatabase
		 *
		 * This class adds some methods for executing queries, providing an interface
		 * different from QSqlDatabase.
		 *
		 * It may also provide methods for managing connection parameters and
		 * abstracions for the differences between different backends (MySQL, SQlite).
		 *
		 * This class uses a QSqlDatabase, and as such is NOT thread safe. More
		 * precisely, this class may only be used in the thread where it was created.
		 * This is a QtSql restriction, see [1].
		 *
		 * [1] http://doc.trolltech.com/4.5/threads.html#threads-and-the-sql-module
		 */
		class DatabaseInterface
		{
			public:
				// *** Data types
				class QueryFailedException
				{
					public:
						QueryFailedException (QSqlQuery query): query (query) {}
						QSqlQuery query;
				};


		    	// *** Constants
				// Data type names like in Rails (use for sk_web) (for MySQL)
				// Not implemented as static constants in order to avoid the static
				// initialization order fiasco. Also, we'll probably want to introduce
				// variable length strings and data types depending on the backend.
		    	static QString dataTypeBinary    ();
		    	static QString dataTypeBoolean   ();
		    	static QString dataTypeDate      ();
		    	static QString dataTypeDatetime  ();
		    	static QString dataTypeDecimal   ();
		    	static QString dataTypeFloat     ();
		    	static QString dataTypeInteger   ();
		    	static QString dataTypeString    ();
		    	static QString dataTypeString16  (); // Non-rails
		    	static QString dataTypeText      ();
		    	static QString dataTypeTime      ();
		    	static QString dataTypeTimestamp ();
		    	static QString dataTypeCharacter (); // Non-Rails
		    	static QString dataTypeId        ();


		    	// *** Construction
				DatabaseInterface ();
				virtual ~DatabaseInterface ();


				// *** Connection management
				bool open (const DatabaseInfo &dbInfo);
				void close ();
				QSqlError lastError () const { return db.lastError (); }
				const DatabaseInfo &getInfo () const { return info; }


				// *** Transactions
				bool transaction () { return db.transaction (); }
				bool commit      () { return db.commit      (); }
				bool rollback    () { return db.rollback    (); }


				// *** Queries
				QSqlQuery prepareQuery (QString queryString, bool forwardOnly=true);
				QSqlQuery &executeQuery (QSqlQuery &query);
				QSqlQuery executeQuery (QString queryString, bool forwardOnly=true);

				// TODO may be more appropriate on a higher level, together with
				// listStrings et al.
				bool queryHasResult (QSqlQuery &query);
				bool queryHasResult (QString queryString);


				// *** Schema manipulation
				void createTable (const QString &name, bool skipIfExists=false);
				void createTableLike (const QString &like, const QString &name, bool skipIfExists=false);
				void dropTable (const QString &name);
				void renameTable (const QString &oldName, const QString &newName);
				bool tableExists (const QString &name);

				void addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="", bool skipIfExists=false);
				void changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="");
				void dropColumn (const QString &table, const QString &name, bool skipIfNotExists=false);
				void renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification="");
				bool columnExists (const QString &table, const QString &name);

				// *** Generic query generation
				static QString selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty=false);
				static QString selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty=false);
				static QString selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty=false);
				static QString selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty=false);


				// *** Generic data manipulation
				void updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue);
				QStringList listStrings (QString queryString);



			private:
				QSqlDatabase db;
				DatabaseInfo info;
		};
	}
}

#endif
