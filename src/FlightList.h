#ifndef _FlightList_h
#define _FlightList_h

/*
 * FlightList
 * martin
 * 2004-10-26
 */

// XXX
#include <q3ptrlist.h>
#include <q3ptrcollection.h>
#define QPtrCollection Q3PtrCollection

#include "src/model/Flight.h"
#include "src/time/sk_time_t.h"

class FlightList:public Q3PtrList<Flight>
{
	public:

	protected:
		virtual int compareItems (QPtrCollection::Item item1, QPtrCollection::Item item2);
};

#endif

