#ifndef _Condition_h
#define _Condition_h

#include <QDateTime>
#include <QString>

#include "src/dataTypes.h"
#include "src/db/dbTypes.h"
#include "src/time/sk_time_t.h"

/*
 * Condition
 * Martin Herrmann
 * 2004-09-09
 */

// Note: when changing this enum, also update Database::make_condition

// TODO rewrite this using named constructors
//
enum ConditionType	// A query condition type
{
	// Generic conditions, applicable to every object type (and
	// producing the same query for every object type...)
	//cond_and, cond_or, cond_not,
	cond_id, cond_any, cond_none,
	// Flight conditions
	cond_flight_pilot, cond_flight_person, cond_flight_plane, cond_flight_plane_or_towplane,	// Crew
	cond_flight_mode_local, cond_flight_mode_coming, cond_flight_mode_going, cond_flight_starts_here,	// Mode
	cond_flight_started, cond_flight_landed, cond_flight_towplane_landed, cond_flight_happened, cond_flight_prepared, 	// Status
	cond_flight_happened_on_date, cond_flight_happened_between, cond_flight_after_date, cond_flight_before_date,	// Date
	cond_flight_started_before, cond_flight_landed_after, cond_flight_start_date_like,	// Given time
	cond_flight_flying_at, cond_flight_will_land,	// Flying
	cond_flight_plane_flying_at, cond_flight_person_flying_at,
	cond_flight_plane_date, cond_flight_person_date, cond_flight_person_date_range,
	// Plane conditions
	cond_plane_registration,
	// Person conditions
	cond_person_first_name, cond_person_last_name, cond_person_name, cond_person_club_club_id
};

class Condition
{
	public:
		// TODO const correctnes
		Condition ();
		Condition (ConditionType _type);
		Condition (ConditionType _type, db_id _id1);
		Condition (ConditionType _type, QString *_text1, QString *_text2=NULL);
		Condition (ConditionType _type, const QDate *_date1, const QDate *_date2=NULL);
		Condition (ConditionType _type, sk_time_t *_given_time1);
		Condition (ConditionType _type, db_id _id1, QDate *_date1, QDate *_date2=NULL);
		void init ();

		// Note: meta queries are not very well tested.
		void set_meta_query (db_object_type _table, const QString *_column);

		db_object_type meta_query_table;
		const QString *meta_query_column;

		ConditionType type;
		db_id id1;
		QString *text1, *text2;
		const QDate *date1, *date2;
		sk_time_t *given_time1;
};

#endif

