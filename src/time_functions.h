#ifndef time_functions_h
#define time_functions_h

#include <string>
#include <time.h>

using namespace std;

enum time_zone { tz_timespan, tz_none, tz_local, tz_utc };

string get_tm_text (struct tm *tm_time, const char *format, int buf_size=0);
string get_current_time_text (time_zone tz, const char *format=NULL, int buf_size=0);


#endif

