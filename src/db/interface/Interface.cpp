#include "Interface.h"

#include <iostream>

#include <QStringList>
#include <QVariant>

#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/util/qString.h"

namespace Db { namespace Interface
{
	// ******************
	// ** Construction **
	// ******************

	Interface::Interface (const DatabaseInfo &dbInfo):
		AbstractInterface (dbInfo)
	{
	}

	Interface::~Interface ()
	{
	}


	// ****************
	// ** Data types **
	// ****************

	// Note: these values are used in migrations. If they are changed, the
	// migrations should be updated to use the same values as before.
	QString Interface::dataTypeBinary    () { return "blob"            ; }
	QString Interface::dataTypeBoolean   () { return "tinyint(1)"      ; }
	QString Interface::dataTypeDate      () { return "date"            ; }
	QString Interface::dataTypeDatetime  () { return "datetime"        ; }
	QString Interface::dataTypeDecimal   () { return "decimal"         ; }
	QString Interface::dataTypeFloat     () { return "float"           ; }
	QString Interface::dataTypeInteger   () { return "int(11)"         ; }
	QString Interface::dataTypeString    () { return "varchar(255)"    ; }
	QString Interface::dataTypeText      () { return "text"            ; }
	QString Interface::dataTypeTime      () { return "time"            ; }
	QString Interface::dataTypeTimestamp () { return "datetime"        ; }
	QString Interface::dataTypeCharacter () { return "varchar(1)"      ; } // Non-Rails
	QString Interface::dataTypeId        () { return dataTypeInteger (); }


	// *************************
	// ** Schema manipulation **
	// *************************

	// TODO more queries in Query
	void Interface::createTable (const QString &name, bool skipIfExists)
	{
		std::cout << QString ("Creating table %1%2")
			.arg (name, skipIfExists?" if it does not exist":"")
			<< std::endl;

//		QString queryString=QString (
//			"CREATE TABLE %1 %2 ("
//			"id int(11) NOT NULL AUTO_INCREMENT,"
//			"PRIMARY KEY (id)"
//			") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
//			)
//			.arg (skipIfExists?"IF NOT EXISTS":"", name);
//
//		executeQuery (queryString);

		executeQuery (Query (
			"CREATE TABLE %1 %2 ("
			"id int(11) NOT NULL AUTO_INCREMENT,"
			"PRIMARY KEY (id)"
			") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
			)
			.arg (skipIfExists?"IF NOT EXISTS":"", name));
	}

	void Interface::createTableLike (const QString &like, const QString &name, bool skipIfExists)
	{
		std::cout << QString ("Creating table %1 like %2%3")
			.arg (name, like, skipIfExists?" if it does not exist":"")
			<< std::endl;

//		QString queryString=
//			QString ("CREATE TABLE %1 %2 LIKE %3")
//			.arg (skipIfExists?"IF NOT EXISTS":"", name, like);
//
//		executeQuery (queryString);

		executeQuery (Query ("CREATE TABLE %1 %2 LIKE %3")
			.arg (skipIfExists?"IF NOT EXISTS":"", name, like));
	}

	void Interface::dropTable (const QString &name)
	{
		std::cout << QString ("Dropping table %1").arg (name) << std::endl;

//		QString queryString=
//			QString ("DROP TABLE %1")
//			.arg (name);
//
//		executeQuery (queryString);

		executeQuery (Query ("DROP TABLE %1").arg (name));
	}

	void Interface::renameTable (const QString &oldName, const QString &newName)
	{
		std::cout << QString ("Renaming table %1 to %2").arg (oldName, newName) << std::endl;

//		QString queryString=
//			QString ("RENAME TABLE %1 TO %2")
//			.arg (oldName, newName);
//
//		executeQuery (queryString);

		executeQuery (Query ("RENAME TABLE %1 TO %2").arg (oldName, newName));
	}

	bool Interface::tableExists (const QString &name)
	{
		// Using addBindValue does not seem to work here
//		QString queryString=
//			QString ("SHOW TABLES LIKE '%1'")
//			.arg (name);
//
//		return queryHasResult (queryString);

		return queryHasResult (Query ("SHOW TABLES LIKE '%1'").arg (name));
	}

	void Interface::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
	{
		if (skipIfExists && columnExists (table, name))
		{
			std::cout << QString ("Skipping existing column %1.%2").arg (table, name) << std::endl;
			return;
		}

		std::cout << QString ("Adding column %1.%2").arg (table, name) << std::endl;

//		QString queryString=
//			QString ("ALTER TABLE %1 ADD COLUMN %2 %3 %4")
//			.arg (table, name, type, extraSpecification);
//
//		executeQuery (queryString);

		executeQuery (Query ("ALTER TABLE %1 ADD COLUMN %2 %3 %4")
			.arg (table, name, type, extraSpecification));
	}

	void Interface::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
	{
		std::cout << QString ("Changing column %1.%2 type to %3")
			.arg (table, name, type) << std::endl;

//		QString queryString=
//			QString ("ALTER TABLE %1 MODIFY %2 %3 %4")
//			.arg (table, name, type, extraSpecification);
//
//		executeQuery (queryString);

		executeQuery (Query ("ALTER TABLE %1 MODIFY %2 %3 %4")
			.arg (table, name, type, extraSpecification));
	}

	void Interface::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
	{
		if (skipIfNotExists && !columnExists (table, name))
		{
			std::cout << QString ("Skipping non-existing column %1.%2").arg (table, name) << std::endl;
			return;
		}

		std::cout << QString ("Dropping column %1.%2").arg (table, name) << std::endl;

//		QString queryString=
//			QString ("ALTER TABLE %1 DROP COLUMN %2")
//			.arg (table, name);
//
//		executeQuery (queryString);

		executeQuery (Query ("ALTER TABLE %1 DROP COLUMN %2").arg (table, name));
	}

	void Interface::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
	{
		std::cout << QString ("Renaming column %1.%2 to %3").arg (table, oldName, newName) << std::endl;

//		QString queryString=
//			QString ("ALTER TABLE %1 CHANGE %2 %3 %4 %5")
//			.arg (table, oldName, newName, type, extraSpecification);
//
//		executeQuery (queryString);

		executeQuery (Query ("ALTER TABLE %1 CHANGE %2 %3 %4 %5")
			.arg (table, oldName, newName, type, extraSpecification));
	}

	bool Interface::columnExists (const QString &table, const QString &name)
	{
//		// Using addBindValue does not seem to work here
//		QString queryString=
//			QString ("SHOW COLUMNS FROM %1 LIKE '%2'")
//			.arg (table, name);
//
//		return queryHasResult (queryString);

		return queryHasResult (Query ("SHOW COLUMNS FROM %1 LIKE '%2'").arg (table, name));
	}


	// *******************************
	// ** Generic data manipulation **
	// *******************************

	void Interface::updateColumnValues (const QString &tableName, const QString &columnName,
		const QVariant &oldValue, const QVariant &newValue)
	{
//		QString queryString=QString ("UPDATE %1 SET %2=? WHERE %2=?")
//			.arg (tableName, columnName);
//
//		QSqlQuery query=prepareQuery (queryString);
//		query.addBindValue (newValue);
//		query.addBindValue (oldValue);
//
//		executeQuery (query);

		// TODO multi-bind or <<
		executeQuery (Query ("UPDATE %1 SET %2=? WHERE %2=?")
			.arg (tableName, columnName).bind (newValue).bind (oldValue));
	}

	QStringList Interface::listStrings (const Query &query)
	{
		QSharedPointer<Result::Result> result=executeQueryResult (query, true);

		QStringList stringList;

		while (result->next ())
			stringList.append (result->value (0).toString ());

		return stringList;
	}

	int Interface::countQuery (const Query &query)
	{
		QSharedPointer<Result::Result> result=executeQueryResult (query, true);

		result->next ();
		return result->value (0).toInt ();
	}

} }
