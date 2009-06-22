#ifndef flight_list_h
#define flight_list_h

/*
 * flight_list
 * martin
 * 2004-10-26
 */

// XXX
#include <q3ptrlist.h>
#include <q3ptrcollection.h>
#define QPtrCollection Q3PtrCollection

#include "src/model/Flight.h"
#include "src/time/sk_time_t.h"

using namespace std;

class flight_list:public Q3PtrList<Flight>
{
	public:

	protected:
		virtual int compareItems (QPtrCollection::Item item1, QPtrCollection::Item item2);
};

#endif

