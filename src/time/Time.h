#ifndef _Time_h
#define _Time_h

#ifndef _SVID_SOURCE
	#define _SVID_SOURCE	// Required for timegm () and timelocal ()
#endif
#ifndef _XOPEN_SOURCE
	#define _XOPEN_SOURCE	// Requried for strptime ()
#endif

#include <cstdio>
#include <ctime>
#include <QString>

#include <QDateTime>

// TODO reduce dependencies
#include "src/logging/messages.h"
#include "src/time/timeFunctions.h"

// TODO: there should be a separate class for duration. Also, this class sucks.
class Time
{
	// TODO: make time_zone parameters non-default - it is important that they
	// are set to the correct values.
	public:
		Time ();	// Default constructor, init to 0
		Time (time_t);	// Contructor, init to given time
		static Time create (const QDateTime &qDateTime, time_zone tz);

		void set_to (const time_t, bool null_sec=false);	// Set to given time
		void set_to (const int, bool null_sec=false);	// Set to given time
		void set_to (const int, const int, const int, const int, const int, const int, time_zone tz=tz_utc);
		void set_to (const QDate, const QTime, /*time_zone tz=tz_utc,*/ bool null_sec=false);
		void parse (const char *, time_zone, const char *);
		void set_current (bool null_sec=false);	// Set to current time
		void null_seconds ();
		void add_seconds (int s);
		void add_time (Time tt);
		void set_null ();	// Set to 0
		bool is_null () const;	// Check for 0
		QTime get_qtime (time_zone tz=tz_utc) const;
		QDate get_qdate (time_zone tz=tz_utc) const;
		QDateTime toUtcQDateTime () const;

		int secs_to (const Time *) const;
		int secs_to (const Time &) const;

		QString to_string (const char *format=NULL, time_zone tz=tz_utc, int buf_size=0, bool no_letters=false) const;	// Generate QString
		QString table_string (const char *, time_zone tz=tz_utc, int buf_size=0, bool no_letters=false) const;
		// TODO: landed ist MURX, no_letters ist schlecht so
		QString table_string (time_zone tz=tz_utc, bool gelandet=true, bool no_letters=false) const;
		QString csv_string (time_zone tz=tz_utc) const;

		bool operator> (const Time &) const;
		bool operator< (const Time &) const;
		bool operator== (const Time &) const;

		//const char *default_format ();
		time_t unix_time () const;
	private:
		time_t t;
		bool null;
};

#endif

