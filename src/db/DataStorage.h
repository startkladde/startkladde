#ifndef _DataStorage_h
#define _DataStorage_h

#include "src/db/Database.h"

/**
 * A wrapper around Database
 *
 * This wrapper provides:
 *   - a better interfaces
 *   - caching
 *
 * This should be considered a temporary solution to facilitate cleanups of the
 * GUI classes, primarily the flight editor window. It will probably be merged
 * with the Database class some time in the future.
 *
 * When using this class, all write accesses to the database should be done
 * using this class. If not, the cache may be out of date. In that case,
 * refresh () must be called between write accesses to the database not using
 * this class and read accesses using this class.
 */
class DataStorage
{
	public:
		DataStorage (Database &db);

		void refresh ();

//		const List<Plane> getPlanes () const { return planes; }
//		const List<Person> getPeople () const { return people; }
//		const List<Flight> getFlightsToday ();

	private:
		Database &db;

//		List<Plane> planes;
//		List<Person> people;
//
//		Date today;
//		List<Flight> flightsToday;
};

#endif /* DATASTORAGE_H_ */
