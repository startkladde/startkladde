#include "DataStorage.h"

// ******************
// ** Construction **
// ******************

DataStorage::DataStorage (Database &db):
	db (db)
{
}

void DataStorage::refresh ()
{

}

//const DataStorage::List<Flight> getFlightsToday ()
//{
//	Date now=Date::current ();
//
//	if (now!=today)
//		refreshFlightsToday ();
//}
