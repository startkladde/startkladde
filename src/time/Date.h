#ifndef _Date_h
#define _Date_h

#include <QString>

#include <QDateTime>

#include "src/accessor.h"

// TODO make gettable from Time, remove functionality here.
class Date
{
	public:
		Date (int _year=0, int _month=0, int _day=0): year (_year), month (_month), day (_day) {}
		static Date current ();

		QString text ();
		bool is_invalid ();

		RW_ACCESSOR (unsigned int, year)
		RW_ACCESSOR (unsigned int, month)
		RW_ACCESSOR (unsigned int, day)

		operator QDate ();	// Transitional, remove

		// Relations
		bool operator== (const Date &o);
		bool operator< (const Date &o);
		bool operator> (const Date &o);
		bool operator<= (const Date &o);
		bool operator>= (const Date &o);

	private:
		unsigned int year, month, day;
};

#endif

