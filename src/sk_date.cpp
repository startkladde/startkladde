#include "sk_date.h"

#include "time.h"
#include "text.h"

sk_date sk_date::current ()/*{{{*/
{
	struct tm lt;
	time_t t=time (NULL);
	// TODO support different time zones
	localtime_r (&t, &lt);

	// Preset default values
	unsigned int day=lt.tm_mday;
	unsigned int month=lt.tm_mon+1;    //} Was hat denn struct tm für
	unsigned int year=lt.tm_year+1900; //} eine saublöde Codierung...

	return sk_date (year, month, day);
}
/*}}}*/

string sk_date::text ()/*{{{*/
{
	return (num_to_string (year, 4)+"-"+num_to_string (month, 2)+"-"+num_to_string (day, 2));
}
/*}}}*/

bool sk_date::is_invalid ()/*{{{*/
{
	return (year==0 || month==0 || day==0);
}
/*}}}*/

sk_date::operator QDate ()/*{{{*/
{
	return QDate (year, month, day);
}
/*}}}*/

bool sk_date::operator== (const sk_date &o)/*{{{*/
{
	return (year==o.year && month==o.month && day==o.day);
}
/*}}}*/

// OK, date comparison:
bool sk_date::operator< (const sk_date &o)/*{{{*/
{
	if (year    < o.year)  return true;
	if (o.year  < year)    return false;
	if (month   < o.month) return true;
	if (o.month < month)   return false;
	if (day     < o.day)   return true;
	if (o.day   < day)     return false;
	return false;	// Dates are equal.
}
/*}}}*/

bool sk_date::operator> (const sk_date &o)/*{{{*/
{
	if (year    > o.year)  return true;
	if (o.year  > year)    return false;
	if (month   > o.month) return true;
	if (o.month > month)   return false;
	if (day     > o.day)   return true;
	if (o.day   > day)     return false;
	return false;	// Dates are equal.
}
/*}}}*/

bool sk_date::operator<= (const sk_date &o)/*{{{*/
{
	if (year    < o.year)  return true;
	if (o.year  < year)    return false;
	if (month   < o.month) return true;
	if (o.month < month)   return false;
	if (day     < o.day)   return true;
	if (o.day   < day)     return false;
	return true;	// Dates are equal.
}
/*}}}*/

bool sk_date::operator>= (const sk_date &o)/*{{{*/
{
	if (year    > o.year)  return true;
	if (o.year  > year)    return false;
	if (month   > o.month) return true;
	if (o.month > month)   return false;
	if (day     > o.day)   return true;
	if (o.day   > day)     return false;
	return true;	// Dates are equal.
}
/*}}}*/

