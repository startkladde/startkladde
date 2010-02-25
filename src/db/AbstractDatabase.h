/*
 * AbstractDatabase.h
 *
 *  Created on: 24.02.2010
 *      Author: Martin Herrmann
 */

#ifndef ABSTRACTDATABASE_H_
#define ABSTRACTDATABASE_H_

#include <QString>
#include <QVariant>
#include <QDate>
#include <QStringList>

class Flight;

namespace Db
{
	class Query;

	/**
	 * A common base class for Database and ThreadSafeDatabase.
	 *
	 * This class provides some common functionality like methods for listing
	 * strings and selecting specific sets of flights.
	 *
	 * Note that the ORM methods are template methods and as such cannot be
	 * declared as pure virtual methods. This means that it is usually not possible
	 * to declare an AbstractDatabase& and use a Database or ThreadSafeDatabase -
	 * the concrete implementation type has to be declared.
	 * This is also the reason why we need to explicitly declare #getFlights and
	 * why its implementation is identical for Database and ThreadSafeDatabase.
	 */
	class AbstractDatabase
	{
		public:
			AbstractDatabase ();
			virtual ~AbstractDatabase ();

			virtual QStringList listStrings (const Query &query)=0;
			virtual QList<Flight> getFlights (const QString &condition="", const QList<QVariant> &conditionValues=QList<QVariant> ())=0;

			virtual QStringList listLocations ();
			virtual QStringList listAccountingNotes ();
			virtual QStringList listClubs ();
			virtual QStringList listPlaneTypes ();
			virtual QList<Flight> getPreparedFlights ();
			virtual QList<Flight> getFlightsDate (QDate date);
	};
}

#endif
