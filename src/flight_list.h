#ifndef flight_list_h
#define flight_list_h

/*
 * flight_list
 * martin
 * 2004-10-26
 */

//Qt3:
//#include <qptrlist.h>
//Qt4:
#include <q3ptrlist.h>
#include <q3ptrcollection.h>
#define QPtrCollection Q3PtrCollection

#include "sk_flug.h"
#include "sk_time_t.h"

using namespace std;

// Qt3:
//class flight_list:public QPtrList<sk_flug>
// Qt4:
class flight_list:public Q3PtrList<sk_flug>
{
	public:

	protected:
		virtual int compareItems (QPtrCollection::Item item1, QPtrCollection::Item item2);
};

#endif

