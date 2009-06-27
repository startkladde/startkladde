#include "FlightList.h"

int FlightList::compareItems (QPtrCollection::Item item1, QPtrCollection::Item item2)
{
	Flight *f1=(Flight *)item1;
	Flight *f2=(Flight *)item2;

	return f1->sort (f2);
}

