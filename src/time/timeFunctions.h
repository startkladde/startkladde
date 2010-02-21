#ifndef TIMEFUNCTIONS_H_
#define TIMEFUNCTIONS_H_

#include <ctime>
#include <QString>

enum time_zone { tz_timespan, tz_none, tz_local, tz_utc };

QString get_tm_text (struct tm *tm_time, const char *format, int buf_size=0);
QString get_current_time_text (time_zone tz, const char *format=NULL, int buf_size=0);


#endif

