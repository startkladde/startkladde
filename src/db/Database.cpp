/*
 * Short term plan:
 *   - read/write flights, people (must be completed, see deconstruction) and planes
 *   - change DataStorage to use this class, it shouldn't use a lot of the fancy old functions
 *   - remove old database
 *   - remove old db_proxy and admin_functions (check no longer needed)
 *   - reenable database checking (?)
 *   - add creating the database
 *
 * Medium term plan:
 *   - add a version number
 *   - add migrations
 *     - one possibility: with create 0, up/down, and create current
 *     - another: autogenerate current from 0 and ups
 *     - look at rails (or other framework supporting migrations)
 *
 * Long term plan:
 *   - use a memory SQLite for local storage (datastorage functionality)
 *   - merge data storage and database methods, so we can use a (local)
 *     database directly (w/o local cache)
 *       - we'd probably have abstract DataStorage and inherited Database and
 *         CachingDataStorage which uses ad Database for access and one (in
 *         memory) for local caching
 *       - question: can we afford using a query on a memory database e. g.
 *         for list of first names rather than maintaining the list explicitly?
 *         should be fast enough (does sqlite memory table have index?)
 *   - local disk caching
 */

#include "Database.h"

#include <iostream>

#include "src/config/Options.h"
#include "src/text.h"

// TODO: objectExists
// TODO: check if the slow part is the loop; if yes, add ProgressMonitor

/*
 * Here's an idea for specifying conditions, or parts thereof:
 * Condition ("foo=? and bar=?") << 42 << "baz";
 */

Database::Database ()
{
    db=QSqlDatabase::addDatabase ("QMYSQL");

    db.setHostName (opts.server);
    db.setDatabaseName (opts.database);
    db.setUserName (opts.username);
    db.setPassword (opts.password);
    db.setPort (opts.port);
}

bool Database::open ()
{
	bool result=db.open ();
	if (!result) return false;

//	QSqlQuery q ("set names latin1", db);

    QSqlQuery query (db);
    query.prepare ("show variables like 'char%'");
    query.exec ();

    while (query.next())
    {
    	QString name=query.value(0).toString ();
    	QString value=query.value(1).toString ();
    	std::cout << QString ("%1=%2").arg (name).arg (value) << std::endl;
    }

	return result;
}

Database::~Database()
{
}

template<class T> QList<T> Database::getObjects ()
{
	QString q="select "+T::selectColumnList ()+" from "+T::dbTableName ();
	QSqlQuery query (q, db);

    return T::createListFromQuery (query);
}

template<class T> int Database::countObjects ()
{
	QString q="select count(*) from "+T::dbTableName ();
	QSqlQuery query (q, db);

	query.next ();
    return query.value (0).toInt ();
}

template<class T> bool Database::objectExists (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("select count(*) from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	query.next ();
	return query.value (0).toInt ()>0;
}

template<class T> T Database::getObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("select "+T::selectColumnList ()+" from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	if (!query.next ()) throw NotFoundException ();

	return T::createFromQuery (query);
}

template<class T> int Database::deleteObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("delete from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	return query.numRowsAffected ();
}

template<class T> db_id Database::createObject (T &object)
{
	QSqlQuery query (db);
	query.prepare ("insert into "+T::dbTableName ()+" "+T::insertValueList ());
	object.bindValues (query);
	query.exec ();

	object.id=query.lastInsertId ().toLongLong ();

	return object.id;
}

template<class T> int Database::updateObject (const T &object)
{
	QSqlQuery query (db);
	query.prepare ("update "+T::dbTableName ()+" set "+object.updateValueList ()+" where id=?");
	object.bindValues (query);
	query.addBindValue (object.id);

	query.exec ();

	return query.numRowsAffected ();
}

// Instantiate the class templates
// Classes have to provide:
//   - ::dbTableName ();
//   - ::QString selectColumnList ();
//   - ::createFromQuery (const QSqlQuery &query);
//   - ::insertValueList ();
//   - ::updateValueList ();
//   - ::bindValues (QSqlQuery &q) const;
//   - ::createListFromQuery (QSqlQuery &query);

#include "src/model/Person.h"

template QList<Person> Database::getObjects           ();
template int           Database::countObjects<Person> ();
template bool          Database::objectExists<Person> (db_id id);
template Person        Database::getObject            (db_id id);
template int           Database::deleteObject<Person> (db_id id);
template db_id         Database::createObject         (Person &object);
template int           Database::updateObject         (const Person &object);
