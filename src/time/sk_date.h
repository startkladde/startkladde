#ifndef sk_date_h
#define sk_date_h

/*
 * sk_date
 * martin
 * 2005-02-07
 */

#include <string>

#include <QDateTime>

#include "src/accessor.h"

using namespace std;

// TODO make gettable from sk_time_t, remove functionality here.
class sk_date
{
	public:
		sk_date (int _year=0, int _month=0, int _day=0): year (_year), month (_month), day (_day) {}
		static sk_date current ();

		string text ();
		bool is_invalid ();

		RW_ACCESSOR (unsigned int, year)
		RW_ACCESSOR (unsigned int, month)
		RW_ACCESSOR (unsigned int, day)

		operator QDate ();	// Transitional, remove

		// Relations
		bool operator== (const sk_date &o);
		bool operator< (const sk_date &o);
		bool operator> (const sk_date &o);
		bool operator<= (const sk_date &o);
		bool operator>= (const sk_date &o);

	private:
		unsigned int year, month, day;
};

#endif

