/*
 * Interface.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "src/db/interface/AbstractInterface.h"

class QVariant;


namespace Db
{
	namespace Interface
	{
		/**
		 * An abstract class that adds some functionality (like schema
		 * manipulation and generic data manipulation) to AbstractInterface.
		 *
		 * Interface implementations should inherit from this class rather than
		 * from AbstractInterface.
		 *
		 * Here's an inheritance diagram for Interface (not all of these
		 * classes are currently implemented, though):
		 *
		 *                       -------------------
		 *                       | AbstractInterface |<----------------.
		 *                        -------------------                  |
		 *                                 ^                           |
		 *                                 |                           |
		 *                            -----------                      |
		 *                           | Interface |                     |
		 *                            -----------                      |
		 *                             ^       ^                       |
		 *                            /         \                      |
		 *             ------------------     ---------------------    |
		 *            | DefaultInterface |   | ThreadSafeInterface |*--'
		 *             ------------------     ---------------------
		 *                  ^      ^
		 *                 /        \
		 *     ----------------    -----------------
		 *    |(MySQLInterface)|  |(SQLiteInterface)|
		 *     ----------------    -----------------
		 */
		class Interface: public AbstractInterface
		{
			public:
				// *** Construction
				Interface (const DatabaseInfo &dbInfo);
				virtual ~Interface ();

		    	// *** Data types
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
		    	static QString dataTypeText      ();
		    	static QString dataTypeTime      ();
		    	static QString dataTypeTimestamp ();
		    	static QString dataTypeCharacter (); // Non-Rails
		    	static QString dataTypeId        ();

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

				// *** Generic data manipulation
				void updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue);
				QStringList listStrings (const Query &query);
				int countQuery (const Query &query);
		};
	}
}

#endif
