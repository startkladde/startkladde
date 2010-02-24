#include "AbstractDatabase.h"

#include "src/model/Flight.h"
#include "src/db/DatabaseInterface.h"

#include "src/model/Person.h"
#include "src/model/Plane.h"

AbstractDatabase::AbstractDatabase ()
{
}

AbstractDatabase::~AbstractDatabase ()
{
}

QStringList AbstractDatabase::listLocations ()
{
	return listStrings (DatabaseInterface::selectDistinctColumnQuery (
		Flight::dbTableName (),
		QStringList () << "departure_location" << "landing_location",
		true));
}

QStringList AbstractDatabase::listAccountingNotes ()
{
	return listStrings (DatabaseInterface::selectDistinctColumnQuery (
		Flight::dbTableName (),
		"accounting_notes",
		true));
}

QStringList AbstractDatabase::listClubs ()
{
	return listStrings (DatabaseInterface::selectDistinctColumnQuery (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		"club",
		true));
}

QStringList AbstractDatabase::listPlaneTypes ()
{
	return listStrings (DatabaseInterface::selectDistinctColumnQuery (
		Plane::dbTableName (),
		"type",
		true));
}


QList<Flight> AbstractDatabase::getPreparedFlights ()
{
	// The correct criterion for prepared flights is:
	// !((starts_here and started) or (lands_here and landed))
	// Resolving the flight mode, we get:
	// !( (local and (started or landed)) or (leaving and started) or (coming and landed) )

	QString condition="!( (mode=? AND (departed OR landed)) OR (mode=? AND departed) OR (mode=? AND landed) )";
	QList<QVariant> conditionValues; conditionValues
		<< Flight::modeToDb (Flight::modeLocal  )
		<< Flight::modeToDb (Flight::modeLeaving)
		<< Flight::modeToDb (Flight::modeComing )
		;

	return getFlights (condition, conditionValues);
}

QList<Flight> AbstractDatabase::getFlightsDate (QDate date)
{
	// The correct criterion for flights on a given date is:
	// (happened and effective_date=that_date)
	// effective_date has to be calculated from takeoff time, landing time,
	// status and mode, which is compilicated. Thus, we select a superset of
	// the flights of that date and filter out the correct flights afterwards.

	// The superset criterion is:
	// (launch_date=that_date or landing_date=that_date)
	// Since the database stores the datetimes, we compare them agains the
	// first and last datetime of the date.

	QDateTime thisMidnight (date,             QTime (0, 0, 0)); // Start of day
	QDateTime nextMidnight (date.addDays (1), QTime (0, 0, 0)); // Start of next day

	QString condition="(departure_time>=? AND landing_time<?) OR (departure_time>=? AND landing_time<?)";
	QList<QVariant> conditionValues; conditionValues
		<< thisMidnight << nextMidnight
		<< thisMidnight << nextMidnight
		;

	QList<Flight> candidates=getFlights (condition, conditionValues);

	// For some of the selected flights, the fact that the takeoff or landing
	// time is on that day may not indicate that the flight actually happened
	// on that day. For example, if a flight is prepared (i. e. not taken off
	// nor landed), or leaving, the times may not be relevant.
	// Thus, we only keep flights which happened and where the effective date
	// is the given date.

	QList<Flight> flights;
	foreach (const Flight &flight, candidates)
		if (flight.happened () && flight.effdatum ()==date)
			flights.append (flight);

	return flights;
}

