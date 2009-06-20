#include "flight_list.h"

int flight_list::compareItems (QPtrCollection::Item item1, QPtrCollection::Item item2)/*{{{*/
{
	sk_flug *f1=(sk_flug *)item1;
	sk_flug *f2=(sk_flug *)item2;

	return f1->sort (f2);
}
/*}}}*/

