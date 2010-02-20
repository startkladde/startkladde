#include "timeFunctions.h"

// TODO move to Date?

//const char *default_format="%0d.%0m.%0Y %0H:%0M:%0S";
const char *default_format="%0Y-%0m-%0d %0H:%0M:%0S";

QString get_tm_text (struct tm *tm_time, const char *format, int buf_size)
	/*
	 * Gets a text representation of a time, essentially like strftime.
	 * Parameters:
	 *   - tm_time: the time
	 *   - format: the format, as accepted by strftime
	 *   - buf-size: 0 if the temporary buffer should be made so large it will
	 *     definitely be enough, >0 to use this buffer size. If >0, it must be
	 *     large enough for the QString to be passed to strftime.
	 * Return value:
	 *   - The formatted time.
	 */
{
	// Allocate buffer
	if (buf_size==0) buf_size=4*strlen (format)+1;
	char text[buf_size];

	// Format text
	strftime (text, buf_size, format, tm_time);

	// Convert text to QString
	return QString (text);
}

QString get_current_time_text (time_zone tz, const char *format, int buf_size)
	/*
	 * Gets a text representation of the current time, essentially like strftime.
	 * Parameters:
	 *   - time_zone: the time zone to use.
	 *   - format, buf_size: passed to get_tm_text.
	 * Return value:
	 *   - The formatted time, with "Z" appended if tz is tz_utc.
	 */
{
	//const int buf_length=20;

	// Get system time
	time_t t; time (&t);

	// Break down time, according to time zone parameter
	struct tm tm_time;
	switch (tz)
	{
		case tz_local: localtime_r (&t, &tm_time); break;
		case tz_utc: gmtime_r (&t, &tm_time); break;
		default: gmtime_r (&t, &tm_time); break;
	}

	if (format==NULL)
	{
		format=default_format;
		buf_size=0;
	}

	QString tt=get_tm_text (&tm_time, format, buf_size);
	if (tz==tz_utc) tt+="Z";
	return tt;
}




