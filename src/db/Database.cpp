#include "Database.h"

#include <iostream>

#include "src/text.h"
#include "src/config/Options.h"

// TODO: objectExists
// TODO: check if the slow part is the loop; if yes, add ProgressMonitor

Database::Database ()
{
    db=QSqlDatabase::addDatabase ("QMYSQL");

    db.setHostName (opts.server);
    db.setDatabaseName (opts.database);
    db.setUserName (opts.username);
    db.setPassword (opts.password);
    db.setPort (opts.port);
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
	std::cout << query.executedQuery () << std::endl;

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
