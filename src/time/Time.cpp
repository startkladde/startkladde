#include "Time.h"

// The default format (what this is used for is not entirely clear).
// TODO need this?
const char *def_format="%0d.%0m.%0Y %0H:%0M:%0S";

Time::Time ()
	/*
	 * Constructs a null Time instance.
	 */
{
	t=0;
	null=true;
}

Time::Time (time_t tme)
	/*
	 * Constructs an Time instance given a unix time.
	 * Paramters:
	 *   - tme: the unis time to set.
	 */
{
	t=tme;
	null=false;
}

Time Time::create (const QDateTime &qDateTime, time_zone tz)
{
	const QDate &d=qDateTime.date ();
	const QTime &t=qDateTime.time ();

	Time time;
	time.set_to (d.year (), d.month (), d.day (), t.hour (), t.minute (), t.second (), tz);
	return time;
}



void Time::set_current (bool null_secs)
	/*
	 * Sets the time to the current system time.
	 * Parameters:
	 *   - null_secs: if true, the seconds are set to 0.
	 */
{
	t=time (NULL);
	null=false;
	if (null_secs) null_seconds ();
	if (t==-1)
	{
		log_error ("Fehler beim Lesen der aktuellen Zeit in sk_time_t::set_current ()");
		set_null ();
	}
}

void Time::set_to (const time_t tme, bool null_secs)
	/*
	 * Sets the time to a given value.
	 * Parameters:
	 *   - tme: the given value.
	 *   - null_secs: if true, the seconds are set to 0.
	 */
{
	if (tme==-1)
	{
		set_null ();
	}
	else
	{
		t=tme;
		null=false;
		if (null_secs) null_seconds ();
	}
}

void Time::set_to (const int tme, bool null_secs)
	/*
	 * Same as above, but takes in integer instead of a time_t.
	 */
{
	// Note: if porting somwhere where time_t stores seconds differently,
	// beware.
	t=(time_t)tme;
	null=false;
	if (null_secs) null_seconds ();
}

void Time::set_to (const int year, const int month, const int day, const int hour, const int minute, const int second, time_zone tz)
	/*
	 * Sets the time to a given value, given the components and a time zone
	 * specification.
	 * Parameters:
	 *   - year, month, day, hour, minute, second: the components of the time.
	 *   - tz: the time zone to interpret the components with.
	 */
{
	struct tm tme;
	memset (&tme, '\0', sizeof (tme));

	tme.tm_year=year-1900;	// this stinks
	tme.tm_mon=month-1;	// tm_mon is 0..11, we want 1..12
	tme.tm_mday=day;
	tme.tm_hour=hour;
	tme.tm_min=minute;
	tme.tm_sec=second;

	switch (tz)
	{
		case tz_local: set_to (timelocal (&tme)); break;
		case tz_utc: set_to (timegm (&tme)); break;
		default: set_to (timegm (&tme)); break;
	}
}

void Time::set_to (const QDate d, const QTime t, /*time_zone tz, */bool null_sec)
	/*
	 * Converts the time from a QDate and a QTime, given a timezone.
	 * Parameters:
	 *   - QDate, QTime: the date and time to set to.
	 *  // - tz: the timezome to interpret the time with.
	 *   - null_sec: if true, the seconds are set to 0.
	 */
{
	set_to (d.year (), d.month (), d.day (), t.hour (), t.minute (), null_sec?0:t.second ());
}

void Time::parse (const char *p, time_zone tz, const char *format)
	/*
	 * Parses a QString and stores the result.
	 * Parameters:
	 *   - p: the QString.
	 *   - tz: the timezone to interpret the QString with.
	 *   - format: the parse format specification.
	 */
{
	struct tm tm_time;
	memset (&tm_time, '\0', sizeof (tm_time));
	strptime (p, format, &tm_time);

	switch (tz)
	{
		case tz_utc: set_to (timegm (&tm_time)); break;
		case tz_local: set_to (timelocal (&tm_time)); break;
		default: set_to (timegm (&tm_time)); break;
	}
}

void Time::null_seconds ()
	/*
	 * Sets the seconds field of the time to 0.
	 */
{
	struct tm broken;
	localtime_r (&t, &broken);
	broken.tm_sec=0;
	t=mktime (&broken);
}

void Time::set_null ()
	/*
	 * Sets the time to the null time.
	 */
{
	t=0;
	null=true;
}

void Time::add_seconds (int s)
	/*
	 * Adds a specified span to the time.
	 * Parameters:
	 *   - s: the span, in seconds.
	 */
{
	t+=s;
	null=false;
}

void Time::add_time (Time tt)
	/*
	 * Adds another time.
	 * Parameters:
	 *   - tt: the time to addObject. This should be a time span rather that a time
	 *     point or the result won't make much sense.
	 */
{
	t+=tt.unix_time ();
	null=false;
}

bool Time::is_null () const
	/*
	 * Finds out if the time is the null time.
	 * Return value:
	 *   - true if the time is null.
	 *   - false else.
	 */
{
	return null;
}

QTime Time::get_qtime (time_zone tz) const
	/*
	 * Makes a QTime from the time.
	 * Parameters:
	 *   - tz: the timezone to construct the QTime for.
	 * Return value:
	 *   the QTime.
	 */
{
	struct tm broken;
	switch (tz)
	{
		case tz_utc: gmtime_r (&t, &broken); break;
		case tz_local: localtime_r (&t, &broken); break;
		default: gmtime_r (&t, &broken); break;
	}
	return QTime (broken.tm_hour, broken.tm_min, broken.tm_sec);
}

QDate Time::get_qdate (time_zone tz) const
	/*
	 * Makes a QDate from the time.
	 * Parameters:
	 *   - tz: the timezone to construct the QTime for.
	 * Return value:
	 *   the QDate.
	 */
{
	if (is_null ()) return QDate ();

	struct tm broken;
	switch (tz)
	{
		case tz_utc: gmtime_r (&t, &broken); break;
		case tz_local: localtime_r (&t, &broken); break;
		default: gmtime_r (&t, &broken); break;
	}
	return QDate (broken.tm_year+1900, broken.tm_mon+1, broken.tm_mday);
}

QDateTime Time::toUtcQDateTime () const
{
	return QDateTime (get_qdate (tz_utc), get_qtime (tz_utc), Qt::UTC);
}



int Time::secs_to (const Time *other) const
	/*
	 * Returns the difference to another time, in seconds.
	 * Parameters:
	 *   - other: the other time.
	 * Return value:
	 *   the difference.
	 */
{
	return (int)difftime (other->t, t);
}

int Time::secs_to (const Time &other) const
	/*
	 * Same as the above function, taking a reference instead of a pointer.
	 */
{
	return secs_to (&other);
}



// TODO change no_letters to letters
QString Time::to_string (const char *format, time_zone tz, int buf_size, bool no_letters) const
	/*
	 * Converts the time to a QString.
	 * Parameters:
	 *   - format: the format specification as understood by the get_tm_text
	 *     function from time_functions.h.
	 *   - tz: the time zone to make the QString for.
	 *   - buf_size: the maximum buffer size needed. Passed on to get_tm_text.
	 *   - no_letters: don't append timezone specification (like Z for GMT).
	 * Return value:
	 *   - the generated QString.
	 */
{
	if (is_null ())
		return "-";

	// Break down time, according to local_time parameter
	struct tm tm_time;
	switch (tz)
	{
		case tz_local: localtime_r (&t, &tm_time); break;
		case tz_utc: gmtime_r (&t, &tm_time); break;
		case tz_timespan: gmtime_r (&t, &tm_time); break;
		default: gmtime_r (&t, &tm_time); break;	// Fehler
	}

	// Set default format if none given
	if (!format) format=def_format;

	QString t=get_tm_text (&tm_time, format, buf_size);
	if (tz==tz_utc && !no_letters) t+="Z";
	return t;
}

// TODO rename
QString Time::csv_string (time_zone tz) const
	/*
	 * Makes a QString suitable for outputting CSV files.
	 * Return value:
	 *   the QString.
	 */
{
	if (is_null ())
		return "";
	else
		return to_string ("%H:%M", tz, 6, true);
}

QString Time::table_string (const char *format, time_zone tz, int buf_size, bool no_letters) const
	/*
	 * Makes a QString suitable for displaying in a Table.
	 * Parameters:
	 *   - format: the format to use. Passed on to to_string.
	 *   - tz: the timezone to generate the QString for.
	 *   - buf_size: the maximum buffer size needed, passed on to to_string.
	 * Return value:
	 *   the QString.
	 */
{
	if (is_null ())
		return "";
	else
		return to_string (format, tz, buf_size, no_letters);
}

QString Time::table_string (time_zone tz, bool gelandet, bool no_letters) const
	/*
	 * Makes a QString suitable for displaying in a Table.
	 * Parameters:
	 *   - tz: the timezone to generate the QString for.
	 *   - landed: whether the flight has landed.
	 */
{
	if (gelandet)
		return table_string ("%H:%M", tz, 6, no_letters);
	else
		return table_string ("(%H:%M)", tz, 8, no_letters);
}


bool Time::operator> (const Time &other) const
	/*
	 * Finds out if a time is later than another time.
	 * Parameters:
	 *   - other: the other time.
	 * Return value:
	 *   - true if this time is later than other.
	 *   - false else.
	 */
{
	return (secs_to (&other)<0);
}

bool Time::operator< (const Time &other) const
	/*
	 * The exact opposite of operator>.
	 */
{
	return (secs_to (&other)>0);
}

bool Time::operator== (const Time &other) const
	/*
	 * Finds out if the time is equal to another.
	 * Parameters:
	 *   - other: the other time.
	 * Return value:
	 *   - true if this time and other are equal.
	 *   - false else.
	 */
{
	return (other.t==t);
}



time_t Time::unix_time () const
	/*
	 * Returns the unix time for this time.
	 * Return value:
	 *   the unix time.
	 */
{
	return t;
}

