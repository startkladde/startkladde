#ifndef condition_t_h
#define condition_t_h

#include "src/data_types.h"

#include "src/time/sk_time_t.h"
#include <string>
#include <qdatetime.h>
#include "src/db/db_types.h"

/*
 * condition_t
 * Martin Herrmann
 * 2004-09-09
 */

using namespace std;

// Note: when changing this enum, also update sk_db::make_condition

// TODO rewrite this using named constructors
//
enum condition_type_t	// A query condition type
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

class condition_t
{
	public:
		// TODO const correctnes
		condition_t ();
		condition_t (condition_type_t _type);
		condition_t (condition_type_t _type, db_id _id1);
		condition_t (condition_type_t _type, string *_text1, string *_text2=NULL);
		condition_t (condition_type_t _type, const QDate *_date1, const QDate *_date2=NULL);
		condition_t (condition_type_t _type, sk_time_t *_given_time1);
		condition_t (condition_type_t _type, db_id _id1, QDate *_date1, QDate *_date2=NULL);
		void init ();

		// Note: meta queries are not very well tested.
		void set_meta_query (db_object_type _table, const string *_column);

		db_object_type meta_query_table;
		const string *meta_query_column;

		condition_type_t type;
		db_id id1;
		string *text1, *text2;
		const QDate *date1, *date2;
		sk_time_t *given_time1;
};

#endif

