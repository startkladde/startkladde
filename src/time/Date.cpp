// FIXME remove
//#include "Date.h"
//
//#include <time.h>
//
//#include "src/text.h"
//
//Date Date::current ()
//{
//	struct tm lt;
//	time_t t=time (NULL);
//	// TODO support different time zones
//	localtime_r (&t, &lt);
//
//	// Preset default values
//	unsigned int day=lt.tm_mday;
//	unsigned int month=lt.tm_mon+1;    //} Was hat denn struct tm für
//	unsigned int year=lt.tm_year+1900; //} eine saublöde Codierung...
//
//	return Date (year, month, day);
//}
//
//QString Date::text ()
//{
//	return (QString::number (year, 4)+"-"+QString::number (month, 2)+"-"+QString::number (day, 2));
//}
//
//bool Date::is_invalid ()
//{
//	return (year==0 || month==0 || day==0);
//}
//
//Date::operator QDate ()
//{
//	return QDate (year, month, day);
//}
//
//bool Date::operator== (const Date &o)
//{
//	return (year==o.year && month==o.month && day==o.day);
//}
//
//// OK, date comparison:
//bool Date::operator< (const Date &o)
//{
//	if (year    < o.year)  return true;
//	if (o.year  < year)    return false;
//	if (month   < o.month) return true;
//	if (o.month < month)   return false;
//	if (day     < o.day)   return true;
//	if (o.day   < day)     return false;
//	return false;	// Dates are equal.
//}
//
//bool Date::operator> (const Date &o)
//{
//	if (year    > o.year)  return true;
//	if (o.year  > year)    return false;
//	if (month   > o.month) return true;
//	if (o.month > month)   return false;
//	if (day     > o.day)   return true;
//	if (o.day   > day)     return false;
//	return false;	// Dates are equal.
//}
//
//bool Date::operator<= (const Date &o)
//{
//	if (year    < o.year)  return true;
//	if (o.year  < year)    return false;
//	if (month   < o.month) return true;
//	if (o.month < month)   return false;
//	if (day     < o.day)   return true;
//	if (o.day   < day)     return false;
//	return true;	// Dates are equal.
//}
//
//bool Date::operator>= (const Date &o)
//{
//	if (year    > o.year)  return true;
//	if (o.year  > year)    return false;
//	if (month   > o.month) return true;
//	if (o.month > month)   return false;
//	if (day     > o.day)   return true;
//	if (o.day   > day)     return false;
//	return true;	// Dates are equal.
//}

