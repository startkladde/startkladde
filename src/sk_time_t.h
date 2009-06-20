#ifndef sk_time_t_h
#define sk_time_t_h

#ifndef _SVID_SOURCE
	#define _SVID_SOURCE	// braucht man für timegm () und timelocal ()
#endif
#ifndef _XOPEN_SOURCE
	#define _XOPEN_SOURCE	// braucht man für strptime ()
#endif

#include <time.h>
#include <string>
#include <cstdio>
#include <qdatetime.h>
#include "messages.h"
#include "time_functions.h"

using namespace std;

class sk_time_t
{
	public:
		sk_time_t ();	// Default constructor, init to 0
		sk_time_t (time_t);	// Contructor, init to given time

		void set_to (const time_t, bool null_sec=false);	// Set to given time
		void set_to (const int, bool null_sec=false);	// Set to given time
		void set_to (const int, const int, const int, const int, const int, const int, time_zone tz=tz_utc);
		void set_to (const QDate, const QTime, time_zone tz=tz_utc, bool null_sec=false);
		void parse (const char *, time_zone, const char *);
		void set_current (bool null_sec=false);	// Set to current time
		void null_seconds ();
		void add_seconds (int s);
		void add_time (sk_time_t tt);
		void set_null ();	// Set to 0
		bool is_null () const;	// Check for 0
		QTime get_qtime (time_zone tz=tz_utc) const;
		QDate get_qdate (time_zone tz=tz_utc) const;

		int secs_to (const sk_time_t *) const;
		int secs_to (const sk_time_t &) const;

		string to_string (const char *format=NULL, time_zone tz=tz_utc, int buf_size=0, bool no_letters=false) const;	// Generate string
		string table_string (const char *, time_zone tz=tz_utc, int buf_size=0, bool no_letters=false) const;
		// TODO: gelandet ist MURX, no_letters ist schlecht so
		string table_string (time_zone tz=tz_utc, bool gelandet=true, bool no_letters=false) const;	
		string csv_string (time_zone tz=tz_utc) const;

		bool operator> (const sk_time_t &) const;
		bool operator< (const sk_time_t &) const;
		bool operator== (const sk_time_t &) const;

		//const char *default_format ();
		time_t unix_time () const;
	private:
		time_t t;
		bool null;
};

#endif

